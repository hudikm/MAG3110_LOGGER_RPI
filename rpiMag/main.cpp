#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"


#include <wiringPi.h>
#include <cstdio>
#include "MAG3110.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <cxxopts.hpp>

MAG3110 mag3110;

std::shared_ptr<spdlog::logger> error_logger;
cxxopts::Options options("rpiMag", "Magnetometer data logger");

/**
 * Pouzite externe kniznice:
 * https://github.com/gabime/spdlog - logger
 * https://github.com/jbeder/yaml-cpp - parser pre yaml subor, zakladna konfuguracia
 * https://github.com/jarro2783/cxxopts - pomocna utilita na spracovanie command line prikazov
 */

int main(int argc, char* argv[])
{
	int16_t x, y, z;
	YAML::Node configFile;
	std::string filename = "default_config.yaml"; // Default file config
	std::string i2c_device_name;
	std::string outsuffix;
	std::string pattern = "{} ,{}, {}";
	std::basic_string<char> outdir = "";
	bool verboseMode = false;
	bool runCalibration = false;
	/*
	 * Command line options
	 */
	options.add_options()
		("d,device", "Name of i2c device(e.g. d=/dev/i2c-3)", cxxopts::value<std::string>())
		("f,file", "Config file, if not set default config file will be used", cxxopts::value<std::string>())
		("o,outsuffix", "Output file suffix", cxxopts::value<std::string>())
		("X,output_dir", "Output directory", cxxopts::value<std::string>())
		("v,verbose", "Print values")
		("c,calibration", "Run with calibration")
		("help", "Print this help");

	auto result = options.parse(argc, argv);

	if (result.count("help"))
	{
		std::cout << options.help({""}) << std::endl;
		exit(0);
	}

	if (result.count("file"))
	{
		filename = result["file"].as<std::string>();
	}

	if (result.count("verbose"))
	{
		verboseMode = true;
	}

	try
	{
		configFile = YAML::LoadFile(filename);
		if (result.count("device"))
		{
			i2c_device_name = result["device"].as<std::string>();
		}
		else
		{
			i2c_device_name = configFile["device"].as<std::string>();
		}

		if (result.count("outsuffix"))
		{
			outsuffix = result["outsuffix"].as<std::string>();
		}
		else
		{
			if (configFile["outsuffix"].IsDefined())
				outsuffix = configFile["outsuffix"].as<std::string>();
			else
			{
				const auto n = i2c_device_name.find("i2c");
				if (n != std::string::npos)
				{
					outsuffix = "_" + i2c_device_name.substr(n);
				}
			}
		}

		if (result.count("output_dir"))
		{
			outdir = result["output_dir"].as<std::string>();
		}
		else
		{
			if (configFile["output_dir"].IsDefined())
				outdir = configFile["output_dir"].as<std::string>();
		}
		
		if (result.count("calibration"))
		{
			runCalibration = true;
		}
		else
		{
			if (configFile["calibration"].IsDefined())
				runCalibration = configFile["calibration"].as<bool>();
		}
	}
	catch (const YAML::RepresentationException& e)
	{
		std::cout << "Error with Yaml file: " << e.msg << std::endl;
		exit(EXIT_FAILURE);
	}

	error_logger = spdlog::basic_logger_mt("error_logger", "error-log" + outsuffix + ".txt");

	// Initialization of wiringPi library
	wiringPiSetupSys();
	mag3110.initialize(i2c_device_name.c_str());
	mag3110.reset();

	if (runCalibration) mag3110.calibration_loop();

	mag3110.start();

	if (!outdir.empty())
	{
		// if windows style of file path is used replace to unix
		std::replace(outdir.begin(), outdir.end(), '\\', '/'); // replace all '\' to '/' 
		// if dir path is not ended with '/' add to end
		const auto ch = outdir.back();
		if (ch != '/')
		{
			outdir += "/";
		}
	}

	auto async_file_path = outdir + "mag_log" + outsuffix + ".txt";
	auto async_file = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", async_file_path);
	if (configFile["pattern"].IsDefined())
	{
		async_file->set_pattern(configFile["pattern"].as<std::string>());
	}

	if (configFile["data_pattern"].IsDefined())
	{
		pattern = configFile["data_pattern"].as<std::string>();
	}
	async_file->info("Calibration:{}, offset x: {}, offset y: {}, offset x: {}", runCalibration,
	                 mag3110.readOffset(MAG3110_X_AXIS), mag3110.readOffset(MAG3110_Y_AXIS),
	                 mag3110.readOffset(MAG3110_Z_AXIS));

	mag3110.setDR_OS(MAG3110_DR_OS_80_16);
	std::cout << "Start mag loop" << std::endl;
	std::cout << "Output: " << async_file_path << " Suffix:" << outsuffix << std::endl;
	while (true)
	{
		mag3110.readMag(&x, &y, &z);
		if (verboseMode) printf("x: %d ,y: %d,z: %d \n\r", x, y, z);
		async_file->info(pattern.c_str(), x, y, z);
		delay(10);
		while (!mag3110.dataReady()) { delayMicroseconds(250); };
	}
	return 0;
}

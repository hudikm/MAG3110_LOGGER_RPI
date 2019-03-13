#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"


#include <wiringPi.h>
#include <cstdio>
#include "MAG3110.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <cxxopts.hpp>
#include "config_vars.h"

MAG3110 mag3110;

std::shared_ptr<spdlog::logger> error_logger;
cxxopts::Options options("rpiMag", "Magnetometer data logger");

/**
 * Pouzite externe kniznice:
 * https://github.com/gabime/spdlog - logger
 * https://github.com/jbeder/yaml-cpp - parser pre yaml subor, zakladna konfuguracia
 * https://github.com/jarro2783/cxxopts - pomocna utilita na spracovanie command line prikazov
 */

void measurement_loop(Config_vars& config_vars)
{
	// Initialization of wiringPi library
	wiringPiSetupSys();
	mag3110.initialize(config_vars.i2c_device_name.c_str());
	mag3110.reset();

	if (config_vars.runCalibration) mag3110.calibration_loop();

	mag3110.start();

	if (!config_vars.outdir.empty())
	{
		// if windows style file path is used change it to unix
		std::replace(config_vars.outdir.begin(), config_vars.outdir.end(), '\\', '/'); // replace all '\' to '/' 
		// if dir path is not ended with '/' add to end
		const auto ch = config_vars.outdir.back();
		if (ch != '/')
		{
			config_vars.outdir += "/";
		}
	}

	auto const async_file_path = config_vars.outdir + "mag_log" + config_vars.outsuffix + ".txt";
	auto async_file = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", async_file_path);
	if (config_vars.configFile["pattern"].IsDefined())
	{
		async_file->set_pattern(config_vars.configFile["pattern"].as<std::string>());
	}

	if (config_vars.configFile["data_pattern"].IsDefined())
	{
		config_vars.pattern = config_vars.configFile["data_pattern"].as<std::string>();
	}
	async_file->info("Calibration:{}, offset x: {}, offset y: {}, offset x: {}", config_vars.runCalibration,
	                 mag3110.readOffset(MAG3110_X_AXIS), mag3110.readOffset(MAG3110_Y_AXIS),
	                 mag3110.readOffset(MAG3110_Z_AXIS));

	mag3110.setDR_OS(MAG3110_DR_OS_80_16);
	std::cout << "Start mag loop" << std::endl;
	std::cout << "Output: " << async_file_path << " Suffix:" << config_vars.outsuffix << std::endl;
	int16_t x;
	int16_t y;
	int16_t z;
	while (true)
	{
		mag3110.readMag(&x, &y, &z);
		if (config_vars.verboseMode) printf("x: %d ,y: %d,z: %d \n\r", x, y, z);
		async_file->info(config_vars.pattern.c_str(), x, y, z);
		delay(10);
		while (!mag3110.dataReady()) { delayMicroseconds(250); };
	}
}

int main(int argc, char* argv[])
{
	// YAML::Node configFile;
	// std::string filename = "default_config.yaml"; // Default file config
	// std::string i2c_device_name = "";
	// std::string outsuffix = "";
	// std::string pattern = "{} ,{}, {}";
	// std::basic_string<char> outdir = "";
	// bool verboseMode = false;
	// bool runCalibration = false;
	Config_vars config_vars;

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

	try
	{
		auto result = options.parse(argc, argv);


		if (result.count("help"))
		{
			std::cout << options.help({""}) << std::endl;
			exit(0);
		}

		if (result.count("file"))
		{
			config_vars.set_filename(result["file"].as<std::string>());
		}

		if (result.count("verbose"))
		{
			config_vars.set_verbose_mode(true);
		}

		try
		{
			config_vars.configFile = YAML::LoadFile(config_vars.get_filename());
			if (result.count("device"))
			{
				config_vars.i2c_device_name = result["device"].as<std::string>();
			}
			else
			{
				config_vars.i2c_device_name = config_vars.configFile["device"].as<std::string>();
			}

			if (result.count("outsuffix"))
			{
				config_vars.outsuffix = result["outsuffix"].as<std::string>();
			}
			else
			{
				if (config_vars.configFile["outsuffix"].IsDefined())
					config_vars.outsuffix = config_vars.configFile["outsuffix"].as<std::string>();
				else
				{
					const auto n = config_vars.i2c_device_name.find("i2c");
					if (n != std::string::npos)
					{
						config_vars.outsuffix = "_" + config_vars.i2c_device_name.substr(n);
					}
				}
			}

			if (result.count("output_dir"))
			{
				config_vars.outdir = result["output_dir"].as<std::string>();
			}
			else
			{
				if (config_vars.configFile["output_dir"].IsDefined())
					config_vars.outdir = config_vars.configFile["output_dir"].as<std::string>();
			}

			if (result.count("calibration"))
			{
				config_vars.runCalibration = true;
			}
			else
			{
				if (config_vars.configFile["calibration"].IsDefined())
					config_vars.runCalibration = config_vars.configFile["calibration"].as<bool>();
			}
		}
		catch (const YAML::RepresentationException& e)
		{
			std::cout << "Error with Yaml file: " << e.msg << std::endl;
			exit(EXIT_FAILURE);
		}
		catch (const YAML::BadFile& e)
		{
			std::cout << e.msg << "\n\r Maybe default config file 'default_config.yaml' is missing ?" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	catch (const cxxopts::OptionParseException& e)
	{
		std::cout << e.what() << "\n\rFor help use --help options" << std::endl;
		exit(EXIT_FAILURE);
	}
	

	error_logger = spdlog::basic_logger_mt("error_logger", "error-log" + config_vars.outsuffix + ".txt");

	// Start measurement loop
	measurement_loop(config_vars);
	return 0;
}

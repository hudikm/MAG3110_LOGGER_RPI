#pragma once
#include <yaml-cpp/node/node.h>

class Config_vars
{
public:
	Config_vars(){};
	~Config_vars(){};
	YAML::Node configFile;
	std::string filename = "default_config.yaml"; // Default file config
	std::string i2c_device_name = "";
	std::string outsuffix = "";
	std::string pattern = "{} ,{}, {}";
	std::basic_string<char> outdir = "";
	bool verboseMode = false;
	bool runCalibration = false;

	YAML::Node& get_config_file()
	{
		return configFile;
	}

	void set_config_file(const YAML::Node& config_file)
	{
		configFile = config_file;
	}

	std::string& get_filename()
	{
		return filename;
	}

	std::string& get_i2_c_device_name()
	{
		return i2c_device_name;
	}

	std::string& get_outsuffix()
	{
		return outsuffix;
	}

	std::string& get_pattern()
	{
		return pattern;
	}

	std::basic_string<char>& get_outdir()
	{
		return outdir;
	}

	bool& is_verbose_mode()
	{
		return verboseMode;
	}

	bool& is_run_calibration()
	{
		return runCalibration;
	}

	// Setters


	Config_vars& set_filename(const std::string& filename)
	{
		this->filename = filename;
		return *this;
	}

	Config_vars& set_i2_c_device_name(const std::string& i2_c_device_name)
	{
		i2c_device_name = i2_c_device_name;
		return *this;
	}

	Config_vars& set_outsuffix(const std::string& outsuffix)
	{
		this->outsuffix = outsuffix;
		return *this;
	}

	Config_vars& set_pattern(const std::string& pattern)
	{
		this->pattern = pattern;
		return *this;
	}

	Config_vars& set_outdir(const std::basic_string<char>& outdir)
	{
		this->outdir = outdir;
		return *this;
	}

	Config_vars& set_verbose_mode(bool verbose_mode)
	{
		verboseMode = verbose_mode;
		return *this;
	}

	Config_vars& set_run_calibration(bool run_calibration)
	{
		runCalibration = run_calibration;
		return *this;
	}
	
	// Constructors
	Config_vars(const Config_vars& other)
		: configFile(other.configFile),
		  filename(other.filename),
		  i2c_device_name(other.i2c_device_name),
		  outsuffix(other.outsuffix),
		  pattern(other.pattern),
		  outdir(other.outdir),
		  verboseMode(other.verboseMode),
		  runCalibration(other.runCalibration)
	{
	}

	Config_vars(Config_vars&& other) noexcept
		: configFile(std::move(other.configFile)),
		  filename(std::move(other.filename)),
		  i2c_device_name(std::move(other.i2c_device_name)),
		  outsuffix(std::move(other.outsuffix)),
		  pattern(std::move(other.pattern)),
		  outdir(std::move(other.outdir)),
		  verboseMode(other.verboseMode),
		  runCalibration(other.runCalibration)
	{
	}

	Config_vars& operator=(const Config_vars& other)
	{
		if (this == &other)
			return *this;
		configFile = other.configFile;
		filename = other.filename;
		i2c_device_name = other.i2c_device_name;
		outsuffix = other.outsuffix;
		pattern = other.pattern;
		outdir = other.outdir;
		verboseMode = other.verboseMode;
		runCalibration = other.runCalibration;
		return *this;
	}

	Config_vars& operator=(Config_vars&& other) noexcept
	{
		if (this == &other)
			return *this;
		configFile = std::move(other.configFile);
		filename = std::move(other.filename);
		i2c_device_name = std::move(other.i2c_device_name);
		outsuffix = std::move(other.outsuffix);
		pattern = std::move(other.pattern);
		outdir = std::move(other.outdir);
		verboseMode = other.verboseMode;
		runCalibration = other.runCalibration;
		return *this;
	}
};

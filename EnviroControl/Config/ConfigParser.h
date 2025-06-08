#pragma once

#include <QString>

namespace Cfg
{

struct WeatherForeCastConfig
{
	QString api_url;
	QString api_key;
	double lat;
	double lon;
	int update_sec;
};

struct DeviceConfig
{
	QString device_name;
	QString device_id;
};

struct DeviceConfigList
{
	std::vector<DeviceConfig> device_cfgs;
};

struct Config
{
	WeatherForeCastConfig forecast_cfg;
	DeviceConfigList device_cfg_list;
};

class ConfigParser
{
public:
	static std::optional<Config> parseConfigFile();
};

}
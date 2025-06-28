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

struct WeatherStationConfig
{
	QString port_name;
	int baud_rate;
	int data_bits;
	int stop_bits;
	bool parity;
};

struct DeviceConfigList
{
	std::vector<DeviceConfig> device_cfgs;
};

struct Config
{
	WeatherForeCastConfig forecast_cfg;
	DeviceConfigList device_cfg_list;
	WeatherStationConfig weather_station_cfg;
	QString rules_cfg_relative_path;
};

class ConfigParser
{
public:
	static std::optional<Config> parseConfigFile();
};

}
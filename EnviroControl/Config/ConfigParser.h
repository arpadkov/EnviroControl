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
	int open_gpio_pin;
	int close_gpio_pin;
};

struct WeatherStationConfig
{
	QString port_name;
	int baud_rate;
	int data_bits;
	int stop_bits;
	bool parity;
	int log_frequencs_sec;
	QString log_file_path;
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
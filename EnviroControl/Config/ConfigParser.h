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
	int reset_time_sec;
	QString open_icon;
	QString close_icon;
	int safety_pos; // 1 = Open, 2 = Close (keep in sync with DevicePosition)
};

struct WeatherStationConfig
{
	QString port_name;
	int baud_rate;
	int data_bits;
	int stop_bits;
	bool parity;
	int log_frequency_sec;
	QString log_file_path;
};

struct IndoorStationConfig
{
	QString python_venv_path;
	int polling_interval_sec;
	int data_gpio_pin;
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
	IndoorStationConfig indoor_station_cfg;
};

class ConfigParser
{
public:
	static std::optional<Config> parseConfigFile();
};

}
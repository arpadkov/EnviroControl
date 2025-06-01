#pragma once

#include <QString>

struct WeatherForeCastConfig
{
	QString api_url;
	QString api_key;
	double lat;
	double lon;
	int update_sec;
};

struct Config
{
	WeatherForeCastConfig forecast_cfg;
};

class ConfigParser
{
public:
	static std::optional<Config> parseConfigFile();
};
#pragma once

#include <QtCore/QObject>

namespace Cfg
{
struct WeatherStationConfig;
}

struct WeatherData
{
	double temperature; // Celsius
	double sun_south;   // kLux
	double sun_east;    // kLux
	double sun_west;    // kLux
	bool twighlight;
	double daylight;    // Lux
	double wind;        // m/s
	bool rain;
};

class WeatherStation : public QObject
{
	Q_OBJECT

public:
	WeatherStation(const Cfg::WeatherStationConfig& cfg, QObject* parent);
	~WeatherStation();

private:
	void configurePort(const Cfg::WeatherStationConfig& cfg);
};
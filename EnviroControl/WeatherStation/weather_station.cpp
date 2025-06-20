#include "WeatherStation.h"

#include "ConfigParser.h"

WeatherStation::WeatherStation(const Cfg::WeatherStationConfig& cfg, QObject* parent) : QObject(parent)
{
	configurePort(cfg);
}

WeatherStation::~WeatherStation()
{
}

void WeatherStation::configurePort(const Cfg::WeatherStationConfig& cfg)
{

}

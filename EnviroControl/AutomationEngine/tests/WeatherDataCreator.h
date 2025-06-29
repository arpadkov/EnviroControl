#include "WeatherStation.h"
#include "IndoorStation.h"

struct WeatherDataCreator
{
	static WeatherData createRainy(QDateTime timestamp)
	{
		WeatherData rainy;
		rainy.temperature = 15.0;
		rainy.sun_south = 0;
		rainy.sun_east = 0;
		rainy.sun_west = 0;
		rainy.twighlight = false;
		rainy.daylight = 100;
		rainy.wind = 0;
		rainy.rain = true;
		rainy.timestamp = timestamp;

		return rainy;
	}

	static WeatherData createWindy(QDateTime timestamp, double wind)
	{
		WeatherData cond;
		cond.temperature = 15.0;
		cond.sun_south = 0;
		cond.sun_east = 0;
		cond.sun_west = 0;
		cond.twighlight = false;
		cond.daylight = 100;
		cond.wind = wind;
		cond.rain = false;
		cond.timestamp = timestamp;

		return cond;
	}

	static IndoorData createIndoorData(QDateTime timestamp, double temperature)
	{
		IndoorData cond;
		cond.temperature = temperature;
		cond.humidity = 10;
		cond.timestamp = timestamp;
		return cond;
	}

};


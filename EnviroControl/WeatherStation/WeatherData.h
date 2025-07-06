#pragma once

#include <QtCore/QDateTime>

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
	QDateTime timestamp;

	QString toDebugString() const
	{
		return QString("Temperature: %1 C\nSun S/E/W : %2/%3/%4 kLux\nTwighlight : %5\nDaylight : %6 Lux\nWind : %7 m/s\nRain : %8")
			.arg(temperature)
			.arg(sun_south)
			.arg(sun_east)
			.arg(sun_west)
			.arg(twighlight ? "Yes" : "No")
			.arg(daylight)
			.arg(wind)
			.arg(rain ? "Yes" : "No");
	};
};
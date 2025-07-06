#pragma once

#include <QtCore/QString>

// Definitions for WeatherData logging

namespace WeatherDataFormat
{
static const QString TIMESTAMP = "timestamp";
static const QString TEMPERATURE = "temperature"; // Celsius
static const QString SUN_SOUTH = "sun_south"; // kLux
static const QString SUN_EAST = "sun_east"; // kLux
static const QString SUN_WEST = "sun_west"; // kLux
static const QString TWILIGHT = "twilight"; // boolean
static const QString DAYLIGHT = "daylight"; // Lux
static const QString WIND = "wind"; // m/s
static const QString RAIN = "rain"; // boolean
}
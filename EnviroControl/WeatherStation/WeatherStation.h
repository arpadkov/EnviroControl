#pragma once

#include <QtCore/QObject>

class WeatherData
{
};

class WeatherStation : public QObject
{
	Q_OBJECT

public:
	WeatherStation(QObject* parent);
	~WeatherStation();
};
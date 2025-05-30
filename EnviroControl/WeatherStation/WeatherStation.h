#pragma once

#include <QtCore/QObject.h>

class WeatherStation : public QObject
{
	Q_OBJECT

public:
	WeatherStation(QObject* parent);
	~WeatherStation();
};
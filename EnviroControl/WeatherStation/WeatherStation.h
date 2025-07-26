#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QObject>
#include <QtSerialPort/QSerialPort>
#include <QtCore/QByteArray>

#include "WeatherDataLogger.h"

namespace Cfg
{
struct WeatherStationConfig;
}

class WeatherData;

class WeatherStation : public QObject
{
	Q_OBJECT

public:
	WeatherStation(const Cfg::WeatherStationConfig& cfg, QObject* parent = nullptr);
	~WeatherStation();

public Q_SLOTS:
	void startReading();
	void stopReading();

Q_SIGNALS:
	void weatherDataReady(const WeatherData& data);
	void errorOccurred(const QString& error);

private:
	void initSerialPort();
	void handleReadyRead();
	std::optional<WeatherData> parseWeatherData(QByteArray data);
	bool compareChecksum(const QByteArray& data) const;

	Cfg::WeatherStationConfig _cfg;

	QSerialPort* _port;
	QByteArray _read_buffer;

	WeatherDataLogger _data_logger;
};
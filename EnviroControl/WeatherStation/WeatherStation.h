#pragma once

#include "WeatherDataLogger.h"
#include "ConfigParser.h"

#include <QtCore/QDateTime>
#include <QtCore/QObject>
#include <QtSerialPort/QSerialPort>
#include <QtCore/QByteArray>


namespace Cfg
{
struct WeatherStationConfig;
}

class WeatherData;

class IWeatherStation : public QObject
{
	Q_OBJECT

public:
	IWeatherStation(const Cfg::WeatherStationConfig& cfg, QObject* parent = nullptr)
		: QObject(parent), _cfg(cfg), _data_logger(cfg.log_file_path, cfg.log_frequency_sec, this)
	{
		connect(this, &IWeatherStation::weatherDataReady, &_data_logger, &WeatherDataLogger::onWeatherDataReady);
	};
	~IWeatherStation()
	{
	};

public Q_SLOTS:
	virtual void startReading() = 0;
	virtual void stopReading() = 0;

Q_SIGNALS:
	void weatherDataReady(const WeatherData& data);
	void errorOccurred(const QString& error);

protected:
	Cfg::WeatherStationConfig _cfg;
	WeatherDataLogger _data_logger;
};

class WeatherStation : public IWeatherStation
{
	Q_OBJECT

public:
	WeatherStation(const Cfg::WeatherStationConfig& cfg, QObject* parent = nullptr);
	~WeatherStation();

public Q_SLOTS:
	void startReading() override;
	void stopReading() override;

private:
	void initSerialPort();
	void handleReadyRead();
	std::optional<WeatherData> parseWeatherData(QByteArray data);
	bool compareChecksum(const QByteArray& data) const;

	QSerialPort* _port = nullptr;
	QByteArray _read_buffer;
};
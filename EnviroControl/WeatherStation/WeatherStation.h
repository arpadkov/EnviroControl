#pragma once

#include <QtCore/QObject>
#include <QtSerialPort/QSerialPort>
#include <QtCore/QByteArray>

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

	void startReading();
	void stopReading();

Q_SIGNALS:
	void weatherDataReady(const WeatherData& data);
	void errorOccurred(const QString& error);

private:
	void configurePort(const Cfg::WeatherStationConfig& cfg);
	void handleReadyRead();
	std::optional<WeatherData> parseWeatherData(const QByteArray& data) const;
	bool compareChecksum(const QByteArray& data) const;

	QSerialPort _port;
	QByteArray _read_buffer;
};
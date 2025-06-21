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

class WeatherStation : public QObject
{
	Q_OBJECT

public:
	WeatherStation(const Cfg::WeatherStationConfig& cfg, QObject* parent = nullptr);
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
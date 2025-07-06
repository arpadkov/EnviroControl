#pragma once

#include "WeatherData.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>

class QString;
class QJsonObject;

class WeatherData;

class WeatherDataLogger : public QObject
{
	Q_OBJECT

public:
	WeatherDataLogger(const QString& file_path, int log_frequency_sec, QObject* parent);
	~WeatherDataLogger();

public Q_SLOTS:
		void onWeatherDataReady(const WeatherData& data);

private Q_SLOTS:
	void logCurrentData();

private:
	void appendToLogFile(const QJsonObject& entry);

	QString _log_file_path;
	QTimer* _log_timer;
	std::optional<WeatherData> _last_logged_data = std::nullopt;

	// For parsing
public:
	static std::vector<WeatherData> parseWeatherDataFromFile(const QString& file_path);
};
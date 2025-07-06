#include "WeatherDataLogger.h"

#include "WeatherDataFormat.h"

#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QFile>

WeatherDataLogger::WeatherDataLogger(const QString& file_path, int log_frequency_sec, QObject* parent) :
	QObject(parent), _log_file_path(file_path)
{
	_log_timer = new QTimer(this);
	_log_timer->setInterval(log_frequency_sec * 1000); // Convert seconds to milliseconds
	connect(_log_timer, &QTimer::timeout, this, &WeatherDataLogger::logCurrentData);
	_log_timer->start();
}

WeatherDataLogger::~WeatherDataLogger()
{
}

void WeatherDataLogger::onWeatherDataReady(const WeatherData& data)
{
	_last_logged_data = data;
}

void WeatherDataLogger::logCurrentData()
{
	if (!_last_logged_data.has_value())
		return;

	QJsonObject entry;
	entry[WeatherDataFormat::TIMESTAMP] = _last_logged_data->timestamp.toString(Qt::ISODate);
	entry[WeatherDataFormat::TEMPERATURE] = _last_logged_data->temperature;
	entry[WeatherDataFormat::SUN_SOUTH] = _last_logged_data->sun_south;
	entry[WeatherDataFormat::SUN_EAST] = _last_logged_data->sun_east;
	entry[WeatherDataFormat::SUN_WEST] = _last_logged_data->sun_west;
	entry[WeatherDataFormat::TWILIGHT] = _last_logged_data->twighlight;
	entry[WeatherDataFormat::DAYLIGHT] = _last_logged_data->daylight;
	entry[WeatherDataFormat::WIND] = _last_logged_data->wind;
	entry[WeatherDataFormat::RAIN] = _last_logged_data->rain;

	appendToLogFile(entry);
	_last_logged_data.reset(); // Clear the last logged data after logging
}

void WeatherDataLogger::appendToLogFile(const QJsonObject& entry)
{
	QFile file(_log_file_path);
	if (!file.open(QIODevice::Append | QIODevice::Text))
	{
		qWarning() << "WeatherDataLogger: Failed to open log file for appending:" << _log_file_path;
		return;
	}

	QTextStream out(&file);
	QJsonDocument doc(entry);
	out << doc.toJson(QJsonDocument::Compact) << "\n"; // Write JSON entry as a single line
	file.close();
}

std::vector<WeatherData> WeatherDataLogger::parseWeatherDataFromFile(const QString& file_path)
{
	std::vector<WeatherData> weather_data_list;
	QFile file(file_path);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning() << "WeatherDataLogger: Failed to open log file for reading:" << file_path;
		return weather_data_list; // Return empty list on failure
	}

	while (!file.atEnd())
	{
		QString line = file.readLine().trimmed();
		if (line.isEmpty())
			continue;

		QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8());
		if (!doc.isObject())
		{
			qWarning() << "WeatherDataLogger: Invalid JSON format in line:" << line;
			continue; // Skip invalid lines
		}

		if (doc.isNull())
		{
			qWarning() << "WeatherDataLogger: Null JSON document in line:" << line;
			continue; // Skip null documents
		}

		QJsonObject obj = doc.object();
		WeatherData data;
		data.timestamp = QDateTime::fromString(obj[WeatherDataFormat::TIMESTAMP].toString(), Qt::ISODate);
		data.temperature = obj[WeatherDataFormat::TEMPERATURE].toDouble();
		data.sun_south = obj[WeatherDataFormat::SUN_SOUTH].toDouble();
		data.sun_east = obj[WeatherDataFormat::SUN_EAST].toDouble();
		data.sun_west = obj[WeatherDataFormat::SUN_WEST].toDouble();
		data.twighlight = obj[WeatherDataFormat::TWILIGHT].toBool();
		data.daylight = obj[WeatherDataFormat::DAYLIGHT].toDouble();
		data.wind = obj[WeatherDataFormat::WIND].toDouble();
		data.rain = obj[WeatherDataFormat::RAIN].toBool();
		weather_data_list.push_back(data);
	}

	file.close();
	return weather_data_list;
}

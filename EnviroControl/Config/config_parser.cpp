#include "ConfigParser.h"

#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace Cfg
{

namespace
{
QString getConfigFile()
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	return path + QDir::separator() + "config.json";
}

std::optional<WeatherForeCastConfig> parseWeatherForecastConfig(const QJsonObject& root_obj)
{
	if (!root_obj.contains("weather_forecast_config") || !root_obj["weather_forecast_config"].isObject())
	{
		qCritical() << "'weather_forecast_config' object not found or is not an object in config file";
		return {};
	}

	QJsonObject weather_obj = root_obj["weather_forecast_config"].toObject();

	WeatherForeCastConfig weather_forecast_cfg;
	if (weather_obj.contains("api_url") && weather_obj["api_url"].isString())
	{
		weather_forecast_cfg.api_url = weather_obj["api_url"].toString();
	}
	else
	{
		qCritical() << "'api_url' not found or is not a string in 'Weather' object of config file:";
		return {};
	}

	if (weather_obj.contains("api_key") && weather_obj["api_key"].isString())
	{
		weather_forecast_cfg.api_key = weather_obj["api_key"].toString();
	}
	else
	{
		qCritical() << "'api_key' not found or is not a string in 'Weather' object of config file";
		return {}; // Return an invalid/empty config
	}

	if (weather_obj.contains("lat") && weather_obj["lat"].isDouble())
	{
		weather_forecast_cfg.lat = weather_obj["lat"].toDouble();
	}
	else
	{
		qCritical() << "'lat' not found or is not a string in 'Weather' object of config file";
		return {}; // Return an invalid/empty config
	}

	if (weather_obj.contains("lon") && weather_obj["lon"].isDouble())
	{
		weather_forecast_cfg.lon = weather_obj["lon"].toDouble();
	}
	else
	{
		qCritical() << "'lon' not found or is not a string in 'Weather' object of config file";
		return {}; // Return an invalid/empty config
	}

	if (weather_obj.contains("update_sec") && weather_obj["update_sec"].isDouble())
	{
		weather_forecast_cfg.update_sec = weather_obj["update_sec"].toDouble();
	}
	else
	{
		qCritical() << "'update_sec' not found or is not a string in 'Weather' object of config file";
		return {}; // Return an invalid/empty config
	}

	return weather_forecast_cfg;
}
}

std::optional<Config> ConfigParser::parseConfigFile()
{
	const QString& config_file_path = getConfigFile();
	QFile config_file(config_file_path);

	if (!config_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qCritical() << "Could not open config file:" << config_file.fileName() << "Reason:" << config_file.errorString();
		return {};
	}

	QByteArray json_data = config_file.readAll();
	config_file.close();

	QJsonParseError parse_error;
	QJsonDocument doc = QJsonDocument::fromJson(json_data, &parse_error);

	if (doc.isNull())
	{
		qCritical() << "Failed to parse JSON from config file:" << config_file_path << "Reason:" << parse_error.errorString();
		return {};
	}

	if (!doc.isObject())
	{
		qCritical() << "JSON root is not an object in config file:" << config_file_path;
		return {};
	}

	QJsonObject root_obj = doc.object();

	const auto& weather_forecast_cfg = parseWeatherForecastConfig(root_obj);

	if (!weather_forecast_cfg)
		return {};

	Config cfg;
	cfg.forecast_cfg = weather_forecast_cfg.value();

	return cfg;
}

}
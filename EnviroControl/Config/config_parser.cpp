#include "ConfigParser.h"

#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace Cfg
{

static const QString CONFIG_FILE_NAME = "config.json";

namespace
{
QString getConfigPath()
{
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString extractString(const QJsonObject& obj, const QString& key)
{
	if (!obj.contains(key) || !obj[key].isString())
		throw std::runtime_error(QString("Key '%1' not found or is not a string in config file").arg(key).toStdString());

	return obj[key].toString();
}

double extractDouble(const QJsonObject& obj, const QString& key)
{
	if (!obj.contains(key) || !obj[key].isDouble())
		throw std::runtime_error(QString("Key '%1' not found or is not a double in config file").arg(key).toStdString());
	return obj[key].toDouble();
}

int extractInt(const QJsonObject& obj, const QString& key)
{
	if (!obj.contains(key) || !obj[key].isDouble())
		throw std::runtime_error(QString("Key '%1' not found or is not a double in config file").arg(key).toStdString());
	return obj[key].toInt();
}

bool extractBool(const QJsonObject& obj, const QString& key)
{
	if (!obj.contains(key) || !obj[key].isBool())
		throw std::runtime_error(QString("Key '%1' not found or is not a boolean in config file").arg(key).toStdString());
	return obj[key].toBool();
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
	weather_forecast_cfg.api_url = extractString(weather_obj, "api_url");
	weather_forecast_cfg.api_key = extractString(weather_obj, "api_key");
	weather_forecast_cfg.lat = extractDouble(weather_obj, "lat");
	weather_forecast_cfg.lon = extractDouble(weather_obj, "lon");
	weather_forecast_cfg.update_sec = extractInt(weather_obj, "update_sec");

	return weather_forecast_cfg;
}

std::optional<DeviceConfigList> parseDeviceConfig(const QJsonObject& root_obj)
{
	if (!root_obj.contains("device_config") || !root_obj["device_config"].isObject())
	{
		qCritical() << "'weather_forecast_config' object not found or is not an object in config file";
		return {};
	}

	QJsonObject device_cfg_obj = root_obj["device_config"].toObject();

	DeviceConfigList config_list;
	if (!device_cfg_obj.contains("devices") || !device_cfg_obj["devices"].isArray())
	{
		qCritical() << "'device_config' object does not contain a 'devices' array.";
		return {};
	}

	QJsonArray devices_array = device_cfg_obj["devices"].toArray();

	// Iterate through the devices array and parse each device object
	for (const QJsonValue& device_value : devices_array)
	{
		if (!device_value.isObject())
		{
			qCritical() << "Found a non-object element in 'devices' array.";
			return {};
		}

		QJsonObject device_obj = device_value.toObject();
		DeviceConfig device_cfg;
		device_cfg.device_id = extractString(device_obj, "id");
		device_cfg.device_name = extractString(device_obj, "name");
		device_cfg.open_gpio_pin = extractInt(device_obj, "open_gpio_pin");
		device_cfg.close_gpio_pin = extractInt(device_obj, "close_gpio_pin");

		// Add the successfully parsed DeviceConfig to the list
		config_list.device_cfgs.push_back(device_cfg);
	}

	if (config_list.device_cfgs.empty())
		qWarning() << "No valid device configurations found in the JSON.";

	return config_list;
}

std::optional<WeatherStationConfig> parseWeatherStationConfig(const QJsonObject& root_obj)
{
	if (!root_obj.contains("weather_station_config") || !root_obj["weather_station_config"].isObject())
	{
		qCritical() << "'weather_station_config' object not found or is not an object in config file";
		return {};
	}

	QJsonObject weather_station_obj = root_obj["weather_station_config"].toObject();

	WeatherStationConfig weather_station_cfg;
	weather_station_cfg.port_name = extractString(weather_station_obj, "port_name");
	weather_station_cfg.baud_rate = extractInt(weather_station_obj, "baud_rate");
	weather_station_cfg.data_bits = extractInt(weather_station_obj, "data_bits");
	weather_station_cfg.stop_bits = extractInt(weather_station_obj, "stop_bits");
	weather_station_cfg.parity = extractBool(weather_station_obj, "parity");
	weather_station_cfg.log_frequency_sec = extractInt(weather_station_obj, "log_frequency_sec");
	weather_station_cfg.log_file_path = extractString(weather_station_obj, "log_file");

	return weather_station_cfg;
}

} // namespace

std::optional<Config> ConfigParser::parseConfigFile()
{
	const QString& config_file_path = getConfigPath() + QDir::separator() + CONFIG_FILE_NAME;
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

	try
	{
		const auto& weather_forecast_cfg = parseWeatherForecastConfig(root_obj);
		const auto& devices_cfg = parseDeviceConfig(root_obj);
		const auto& weather_station_cfg = parseWeatherStationConfig(root_obj);
		QString rules_cfg_file = extractString(root_obj, "rules_config_file");

		Config cfg;
		cfg.forecast_cfg = weather_forecast_cfg.value();
		cfg.device_cfg_list = devices_cfg.value();
		cfg.weather_station_cfg = weather_station_cfg.value();
		cfg.rules_cfg_relative_path = getConfigPath() + QDir::separator() + rules_cfg_file;

		return cfg;
	}
	catch (const std::runtime_error& e)
	{
		qCritical() << "Error parsing config file:" << e.what();
		return {};
	}
}

}
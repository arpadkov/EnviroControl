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

WeatherForeCastConfig parseWeatherForecastConfig(const QJsonObject& root_obj, const QString& obj_name)
{
	if (!root_obj.contains(obj_name) || !root_obj[obj_name].isObject())
		throw std::runtime_error(QString("%1 object not found or is not an object in config file").arg(obj_name).toStdString());

	QJsonObject weather_obj = root_obj[obj_name].toObject();

	WeatherForeCastConfig weather_forecast_cfg;
	weather_forecast_cfg.api_url = extractString(weather_obj, "api_url");
	weather_forecast_cfg.api_key = extractString(weather_obj, "api_key");
	weather_forecast_cfg.lat = extractDouble(weather_obj, "lat");
	weather_forecast_cfg.lon = extractDouble(weather_obj, "lon");
	weather_forecast_cfg.update_sec = extractInt(weather_obj, "update_sec");

	return weather_forecast_cfg;
}

DeviceConfigList parseDeviceConfig(const QJsonObject& root_obj, const QString& obj_name)
{
	if (!root_obj.contains(obj_name) || !root_obj[obj_name].isObject())
		throw std::runtime_error(QString("%1 object not found or is not an object in config file").arg(obj_name).toStdString());

	QJsonObject device_cfg_obj = root_obj[obj_name].toObject();

	DeviceConfigList config_list;
	if (!device_cfg_obj.contains("devices") || !device_cfg_obj["devices"].isArray())
		throw std::runtime_error("'device_config' object does not contain a 'devices' array.");

	QJsonArray devices_array = device_cfg_obj["devices"].toArray();

	// Iterate through the devices array and parse each device object
	for (const QJsonValue& device_value : devices_array)
	{
		if (!device_value.isObject())
			throw std::runtime_error("Found a non-object element in 'devices' array.");

		QJsonObject device_obj = device_value.toObject();
		DeviceConfig device_cfg;
		device_cfg.device_id = extractString(device_obj, "id");
		device_cfg.device_name = extractString(device_obj, "name");
		device_cfg.open_gpio_pin = extractInt(device_obj, "open_gpio_pin");
		device_cfg.close_gpio_pin = extractInt(device_obj, "close_gpio_pin");
		device_cfg.reset_time_sec = extractInt(device_obj, "reset_time_sec");
		device_cfg.open_icon = extractString(device_obj, "open_icon");
		device_cfg.close_icon = extractString(device_obj, "close_icon");
		device_cfg.safety_pos = extractInt(device_obj, "safety_pos");

		// Add the successfully parsed DeviceConfig to the list
		config_list.device_cfgs.push_back(device_cfg);
	}

	if (config_list.device_cfgs.empty())
		qWarning() << "No valid device configurations found in the JSON.";

	return config_list;
}

WeatherStationConfig parseWeatherStationConfig(const QJsonObject& root_obj, const QString& obj_name)
{
	if (!root_obj.contains(obj_name) || !root_obj[obj_name].isObject())
		throw std::runtime_error(QString("%1 object not found or is not an object in config file").arg(obj_name).toStdString());

	QJsonObject weather_station_obj = root_obj[obj_name].toObject();

	WeatherStationConfig weather_station_cfg;
	weather_station_cfg.port_name = extractString(weather_station_obj, "port_name");
	weather_station_cfg.baud_rate = extractInt(weather_station_obj, "baud_rate");
	weather_station_cfg.data_bits = extractInt(weather_station_obj, "data_bits");
	weather_station_cfg.stop_bits = extractInt(weather_station_obj, "stop_bits");
	weather_station_cfg.parity = extractBool(weather_station_obj, "parity");
	weather_station_cfg.log_frequency_sec = extractInt(weather_station_obj, "log_frequency_sec");
	weather_station_cfg.log_file_path = getConfigPath() + QDir::separator() + extractString(weather_station_obj, "log_file");

	return weather_station_cfg;
}

IndoorStationConfig parseIndoorStationConfig(const QJsonObject& root_obj, const QString& obj_name)
{
	if (!root_obj.contains(obj_name) || !root_obj[obj_name].isObject())
		throw std::runtime_error(QString("%1 object not found or is not an object in config file").arg(obj_name).toStdString());

	QJsonObject indoor_station_obj = root_obj[obj_name].toObject();

	IndoorStationConfig indoor_station_cfg;
	indoor_station_cfg.python_venv_path = extractString(indoor_station_obj, "python_venv_path");
	indoor_station_cfg.polling_interval_sec = extractInt(indoor_station_obj, "polling_interval_sec");
	indoor_station_cfg.data_gpio_pin = extractInt(indoor_station_obj, "data_gpio_pin");

	return indoor_station_cfg;
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
		Config cfg;
		cfg.forecast_cfg = parseWeatherForecastConfig(root_obj, "weather_forecast_config");
		cfg.device_cfg_list = parseDeviceConfig(root_obj, "device_config");
		cfg.weather_station_cfg = parseWeatherStationConfig(root_obj, "weather_station_config");
		cfg.rules_cfg_relative_path = getConfigPath() + QDir::separator() + extractString(root_obj, "rules_config_file");
		cfg.indoor_station_cfg = parseIndoorStationConfig(root_obj, "indoor_station_cfg");

		return cfg;
	}
	catch (const std::runtime_error& e)
	{
		qCritical() << "Error parsing config file:" << e.what();
		return {};
	}
}

}
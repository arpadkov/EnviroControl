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

		// Extract "id"
		if (device_obj.contains("id") && device_obj["id"].isString())
		{
			device_cfg.device_id = device_obj["id"].toString();
		}
		else
		{
			qCritical() << "Device object missing 'id' or 'id' is not a string";
			return {};
		}

		// Extract "name"
		if (device_obj.contains("name") && device_obj["name"].isString())
		{
			device_cfg.device_name = device_obj["name"].toString();
		}
		else
		{
			qCritical() << "Device object with ID '" << device_cfg.device_id << "' missing 'name' or 'name' is not a string.";
			return {};
		}

		// Extract "open_gpio_pin"
		if (device_obj.contains("open_gpio_pin") && device_obj["open_gpio_pin"].isDouble())
		{
			device_cfg.open_gpio_pin = device_obj["open_gpio_pin"].toInt();
		}
		else
		{
			qCritical() << "Device object with ID '" << device_cfg.device_id << "' missing 'name' or 'name' is not a string.";
			return {};
		}

		// Extract "close_gpio_pin"
		if (device_obj.contains("close_gpio_pin") && device_obj["close_gpio_pin"].isDouble())
		{
			device_cfg.close_gpio_pin = device_obj["close_gpio_pin"].toInt();
		}
		else
		{
			qCritical() << "Device object with ID '" << device_cfg.device_id << "' missing 'name' or 'name' is not a string.";
			return {};
		}

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
	if (weather_station_obj.contains("port_name") && weather_station_obj["port_name"].isString())
	{
		weather_station_cfg.port_name = weather_station_obj["port_name"].toString();
	}
	else
	{
		qCritical() << "'port_name' not found or is not a string in 'WeatherStation' object of config file:";
		return {};
	}

	if (weather_station_obj.contains("baud_rate") && weather_station_obj["baud_rate"].isDouble())
	{
		weather_station_cfg.baud_rate = weather_station_obj["baud_rate"].toDouble();
	}
	else
	{
		qCritical() << "'baud_rate' not found or is not a string in 'WeatherStation' object of config file:";
		return {};
	}

	if (weather_station_obj.contains("data_bits") && weather_station_obj["data_bits"].isDouble())
	{
		weather_station_cfg.data_bits = weather_station_obj["data_bits"].toDouble();
	}
	else
	{
		qCritical() << "'data_bits' not found or is not a string in 'WeatherStation' object of config file:";
		return {};
	}

	if (weather_station_obj.contains("stop_bits") && weather_station_obj["stop_bits"].isDouble())
	{
		weather_station_cfg.stop_bits = weather_station_obj["stop_bits"].toDouble();
	}
	else
	{
		qCritical() << "'stop_bits' not found or is not a string in 'WeatherStation' object of config file:";
		return {};
	}

	if (weather_station_obj.contains("parity") && weather_station_obj["parity"].isBool())
	{
		weather_station_cfg.parity = weather_station_obj["parity"].toBool();
	}
	else
	{
		qCritical() << "'parity' not found or is not a string in 'WeatherStation' object of config file:";
		return {};
	}

	if (weather_station_obj.contains("log_frequencs_sec") && weather_station_obj["log_frequencs_sec"].isDouble())
	{
		weather_station_cfg.log_frequencs_sec = weather_station_obj["log_frequencs_sec"].toInt();
	}
	else
	{
		qCritical() << "'log_frequencs_sec' not found or is not a string in 'WeatherStation' object of config file:";
		return {};
	}

	if (weather_station_obj.contains("log_file") && weather_station_obj["log_file"].isString())
	{
		weather_station_cfg.log_file_path = getConfigPath() + QDir::separator() + weather_station_obj["log_file"].toString();
	}
	else
	{
		qCritical() << "'log_file' not found or is not a string in 'WeatherStation' object of config file:";
		return {};
	}

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

	const auto& weather_forecast_cfg = parseWeatherForecastConfig(root_obj);
	const auto& devices_cfg = parseDeviceConfig(root_obj);
	const auto& weather_station_cfg = parseWeatherStationConfig(root_obj);

	QString rules_cfg_file;
	if (root_obj.contains("rules_config_file") && root_obj["rules_config_file"].isString())
	{
		rules_cfg_file = root_obj["rules_config_file"].toString();
	}
	else
	{
		qCritical() << "'rules_config_file' object not found or is not an object in config file";
		return {};
	}

	if (!weather_forecast_cfg || !devices_cfg || !weather_station_cfg || rules_cfg_file.isEmpty())
		return {};

	Config cfg;
	cfg.forecast_cfg = weather_forecast_cfg.value();
	cfg.device_cfg_list = devices_cfg.value();
	cfg.weather_station_cfg = weather_station_cfg.value();
	cfg.rules_cfg_relative_path = getConfigPath() + QDir::separator() + rules_cfg_file;

	return cfg;
}

}
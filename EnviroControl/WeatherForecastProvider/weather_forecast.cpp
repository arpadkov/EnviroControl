#include "WeatherForecast.h"
#include "ErrorDetail.h"
#include "ForecastData.h"

#include <QtCore/QUrlQuery>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QStandardPaths>

#include <moc_WeatherForecast.cpp>

namespace WFP
{
const QString WEATHER_API_URL = "https://api.openweathermap.org/data/3.0/onecall";

namespace
{

std::pair<QString, QString> parseConfig()
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	auto config_file_path = path + QDir::separator() + "config.json";
	QFile config_file(config_file_path);

	// Try to open the file in read-only text mode
	if (!config_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning() << "Error: Could not open config file:" << config_file.fileName() << "Reason:" << config_file.errorString();
		return {}; // Return an invalid/empty config
	}

	// Read all content from the file
	QByteArray json_data = config_file.readAll();
	config_file.close(); // Close the file after reading

	// Parse the JSON data
	QJsonParseError parse_error;
	QJsonDocument doc = QJsonDocument::fromJson(json_data, &parse_error);

	// Check for parsing errors
	if (doc.isNull())
	{
		qWarning() << "Error: Failed to parse JSON from config file:" << config_file_path << "Reason:" << parse_error.errorString();
		return {}; // Return an invalid/empty config
	}

	// Ensure the root of the JSON document is an object
	if (!doc.isObject())
	{
		qWarning() << "Error: JSON root is not an object in config file:" << config_file_path;
		return {}; // Return an invalid/empty config
	}

	QJsonObject root_obj = doc.object(); // Get the root JSON object

	// Check if the "Weather" key exists and is an object
	if (!root_obj.contains("WeatherForecastConfig") || !root_obj["WeatherForecastConfig"].isObject())
	{
		qWarning() << "Error: 'Weather' object not found or is not an object in config file:" << config_file_path;
		return {}; // Return an invalid/empty config
	}

	QJsonObject weather_obj = root_obj["WeatherForecastConfig"].toObject(); // Get the "Weather" object

	// Extract "ApiUrl"
	QString api_url;
	if (weather_obj.contains("ApiUrl") && weather_obj["ApiUrl"].isString())
	{
		api_url = weather_obj["ApiUrl"].toString();
	}
	else
	{
		qWarning() << "Error: 'ApiUrl' not found or is not a string in 'Weather' object of config file:" << config_file_path;
		return {}; // Return an invalid/empty config
	}

	// Extract "ApiKey"
	QString api_key;
	if (weather_obj.contains("ApiKey") && weather_obj["ApiKey"].isString())
	{
		api_key = weather_obj["ApiKey"].toString();
	}
	else
	{
		qWarning() << "Error: 'ApiKey' not found or is not a string in 'Weather' object of config file:" << config_file_path;
		return {}; // Return an invalid/empty config
	}

	qDebug() << "Successfully read Weather API configuration from:" << config_file_path;
	return { api_url, api_key }; // Return the populated config struct
}
}


WeatherForecast::WeatherForecast(std::pair<double, double> coordinates, int update_sec, QObject* parent)
	: QObject(parent), _coordinates(coordinates), _update_interval_sec(update_sec)
{
	_fetch_timer = new QTimer(this);
	connect(_fetch_timer, &QTimer::timeout, this, &WeatherForecast::onFetchTimeout);

	auto config = parseConfig();
	_api_url = config.first;
	_api_key = config.second;
}

void WeatherForecast::startFetching()
{
	if (_network_manager)
		return; // Already started fetching

	if (_api_url.isEmpty() || _api_key.isEmpty())
	{
		Q_EMIT errorOccurred("API URL or API key is not set. Please check your configuration.");
		return; // Not initialized properly
	}

	// Create NetworkManager only once moved to target thread
	_network_manager = new QNetworkAccessManager(this);
	connect(_network_manager, &QNetworkAccessManager::finished, this, &WeatherForecast::onNetworkReplyFinished);

	_fetch_timer->setInterval(_update_interval_sec * 1000);
	_fetch_timer->start();

	// Trigger an immediate first fetch
	onFetchTimeout();
}

void WeatherForecast::onFetchTimeout()
{

	if (!_network_manager)
	{
		Q_EMIT errorOccurred("Network manager not initialized. Call startFetching() first.");
		return; // Not initialized
	}

	QUrl url(_api_url);
	QUrlQuery query;

	query.addQueryItem("lat", QString::number(_coordinates.first, 'f', 6));
	query.addQueryItem("lon", QString::number(_coordinates.second, 'f', 6));
	query.addQueryItem("appid", _api_key);
	query.addQueryItem("units", "metric"); // Use metric units by default
	query.addQueryItem("exclude", "minutely,hourly"); // Exclude unnecessary data

	url.setQuery(query);

	qDebug() << "WeatherForecast: Fetching new data:";
	qDebug() << "URL:" << url.toString();

	QNetworkRequest request(url);
	_network_manager->get(request);
}

void WeatherForecast::onNetworkReplyFinished(QNetworkReply* reply)
{
	if (!reply)
	{
		Q_EMIT errorOccurred("Network reply is null.");
		return;
	}

	if (reply->error() != QNetworkReply::NoError)
	{
		Q_EMIT errorOccurred(reply->errorString());
		reply->deleteLater();
		return;
	}

	QByteArray response_data = reply->readAll();
	reply->deleteLater();
	qDebug() << "WeatherForecast: Received response data:" << response_data;

	QJsonDocument json_doc = QJsonDocument::fromJson(response_data);
	if (json_doc.isNull())
	{
		Q_EMIT errorOccurred("Failed to parse JSON response.");
		return;
	}

	QJsonObject json_obj = json_doc.object();
	if (json_obj.isEmpty())
	{
		Q_EMIT errorOccurred("Empty JSON response.");
		return;
	}

	qDebug() << "Weather data fetched successfully:" << json_obj;
	ForecastData data(json_obj);
	//auto data = ForecastData(); // TODO parse from json_obj
	Q_EMIT forecastDataReady(data);
}

}
#include "WeatherForecast.h"
#include "ErrorDetail.h"
#include "ForecastData.h"

#include <QtCore/QUrlQuery>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <moc_WeatherForecast.cpp>

namespace WFP
{

WeatherForecast::WeatherForecast(const Cfg::WeatherForeCastConfig& cfg, QObject* parent)
	: QObject(parent), _cfg(cfg)
{
	_fetch_timer = new QTimer(this);
	connect(_fetch_timer, &QTimer::timeout, this, &WeatherForecast::onFetchTimeout);
}

void WeatherForecast::startFetching()
{
	if (_network_manager)
		return; // Already started fetching

	// Create NetworkManager only once moved to target thread
	_network_manager = new QNetworkAccessManager(this);
	connect(_network_manager, &QNetworkAccessManager::finished, this, &WeatherForecast::onNetworkReplyFinished);

	_fetch_timer->setInterval(_cfg.update_sec * 1000);
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

	QUrl url(_cfg.api_url);
	QUrlQuery query;

	query.addQueryItem("lat", QString::number(_cfg.lat, 'f', 6));
	query.addQueryItem("lon", QString::number(_cfg.lon, 'f', 6));
	query.addQueryItem("appid", _cfg.api_key);
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
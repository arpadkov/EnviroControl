#pragma once

#include "ConfigParser.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QNetworkAccessManager>
#include <QtCore/QPointer>

class QString;
class QNetworkReply;

class ErrorDetail;

namespace WFP
{
class ForecastData;
}


namespace WFP
{

class WeatherForecast : public QObject
{
	Q_OBJECT
public:
	explicit WeatherForecast(const WeatherForeCastConfig& cfg, QObject* parent = nullptr);

public Q_SLOTS:
	void startFetching();

Q_SIGNALS:
	void forecastDataReady(const ForecastData& data);
	void errorOccurred(const ErrorDetail& error);

private Q_SLOTS:
	void onFetchTimeout();
	void onNetworkReplyFinished(QNetworkReply* reply);

private:
	QPointer<QTimer> _fetch_timer = nullptr;
	QPointer<QNetworkAccessManager> _network_manager = nullptr; // create in the thread its used in
	QString _api_key;
	QString _api_url;
	std::pair<double, double> _coordinates; // latitude, longitude
	int _update_interval_sec;
};

}
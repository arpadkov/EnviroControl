#include "WeatherStation.h"

IWeatherStation::IWeatherStation(const Cfg::WeatherStationConfig& cfg, QObject* parent)
	: QObject(parent), _cfg(cfg), _data_logger(cfg.log_file_path, cfg.log_frequency_sec, this)
{
	connect(this, &IWeatherStation::weatherDataReady, &_data_logger, &WeatherDataLogger::onWeatherDataReady);

	connect(this, &IWeatherStation::weatherDataReady, [this](const WeatherData& data)
		{
			if (_watchdog)
				_watchdog->start(); // Restart the watchdog timer on new data
			else
				qWarning() << "(WeatherStation): Watchdog timer is not initialized, cannot start it.";
		});

	initWatchdog();
}

IWeatherStation::~IWeatherStation()
{
	if (_watchdog)
		_watchdog->stop();
}

/*
* The watchdog is started every time a data ready is received. The next one has to arrive before the timeout.
*/
void IWeatherStation::initWatchdog()
{
	_watchdog = new QTimer(this);
	int timeout_sec = _cfg.watchdog_timeout_sec;
	_watchdog->setInterval(timeout_sec * 1000);
	_watchdog->setSingleShot(true);
	connect(_watchdog, &QTimer::timeout, this, [this, timeout_sec]()
		{
			qWarning() << "(WeatherStation): Timeout exceeed, not weather data received in the last " << timeout_sec << " seconds";
			Q_EMIT errorOccurred(QString("Timeout exceeed, not weather data received in the last %1 seconds").arg(timeout_sec));
			stopReading();

		});
}
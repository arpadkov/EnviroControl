#include "MainWindow.h"
#include "ErrorDetail.h"
#include "ForecastData.h"
#include "WeatherForecast.h"
#include "Logging.h"
#include "AutomationEngine.h"
#include "AutomationWidget.h"
#include "WeatherStation.h"
#include "WeatherStationMock.h"

#include <QtCore/QThread>

MainWindow::MainWindow(const Cfg::Config& cfg, QWidget* parent)
	: QMainWindow(parent), _cfg(cfg)
	, ui(new Ui::MainWindowClass())
{
	ui->setupUi(this);

	initWeatherForecastThread();
	initAutomationEngine();
	initWeatherStationThread();
	initIndoorStationThread();
}

MainWindow::~MainWindow()
{
	delete ui;

	// Clean up weatherforecast
	_weather_forecast_thread->exit();
	_weather_forecast_thread->wait();
}

void MainWindow::onWeatherForecastData(const WFP::ForecastData& data)
{
	ui->_test_l->setText(data.toString());
}

void MainWindow::onWeatherData(const WeatherData& data)
{
	ui->_weather_station_label->setText(data.toDebugString());
}

void MainWindow::initWeatherForecastThread()
{
	_weather_forecast_thread = new QThread();
	auto forecast = new WFP::WeatherForecast(_cfg.forecast_cfg);
	forecast->moveToThread(_weather_forecast_thread);

	// Start & finish signals
	QObject::connect(_weather_forecast_thread, &QThread::started, forecast, &WFP::WeatherForecast::startFetching);
	QObject::connect(_weather_forecast_thread, &QThread::finished, forecast, &QObject::deleteLater);

	QObject::connect(forecast, &WFP::WeatherForecast::forecastDataReady, [](const WFP::ForecastData& data)
		{
			qDebug(main_win_log) << "Weather forecast ready:" << data.toString();
		});
	QObject::connect(forecast, &WFP::WeatherForecast::forecastDataReady, this, &MainWindow::onWeatherForecastData);

	QObject::connect(forecast, &WFP::WeatherForecast::errorOccurred, [](const ErrorDetail& error)
		{
			qDebug(main_win_log) << "Error in weather forecast:" << error.getErrorMessage();
		});

	_weather_forecast_thread->start();
}

void MainWindow::initWeatherStationThread()
{
	_weather_station_thread = new QThread();
	IWeatherStation* weather_station = nullptr;
	if (_cfg.weather_station_cfg.port_name.isEmpty())
	{
	// Create MockWeatherStation, if Port config is empty
		qDebug(main_win_log) << "Weather station port name is empty, using mock weather station.";
		weather_station = new WeatherStationMock(_cfg.weather_station_cfg);
	}
	else
	{
		qDebug(main_win_log) << "Weather station port name is set, using real WeatherStation.";
		weather_station = new WeatherStation(_cfg.weather_station_cfg);
	}

	weather_station->moveToThread(_weather_station_thread);

	// Start & finish signals
	QObject::connect(_weather_station_thread, &QThread::started, weather_station, &IWeatherStation::startReading);
	QObject::connect(_weather_station_thread, &QThread::finished, weather_station, &QObject::deleteLater);

	// Notify AutomationEngine when weather data is ready
	QObject::connect(weather_station, &IWeatherStation::weatherDataReady, _automation_engine, &Automation::AutomationEngine::onWeatherStationData);
	QObject::connect(weather_station, &IWeatherStation::errorOccurred, _automation_engine, [this, weather_station](const QString& error)
		{
			weather_station->stopReading(); // Stop reading to avoid further errors
			weather_station->disconnect(); // Disconnect to avoid multiple error signals
			_automation_engine->onError(error);
		});

	QObject::connect(weather_station, &IWeatherStation::weatherDataReady, this, &MainWindow::onWeatherData);
	QObject::connect(weather_station, &IWeatherStation::errorOccurred, this, [this](const QString& error)
		{
			ui->_weather_station_label->setText(error);
		});

	_weather_station_thread->start();
}

void MainWindow::initIndoorStationThread()
{
	_indoor_station_thread = new QThread();
	auto indoor_station = new IndoorStation(_cfg.indoor_station_cfg);
	indoor_station->moveToThread(_indoor_station_thread);

	// Start & finish signals
	QObject::connect(_indoor_station_thread, &QThread::started, indoor_station, &IndoorStation::startReading);
	QObject::connect(_indoor_station_thread, &QThread::finished, indoor_station, &QObject::deleteLater);

	// Notify AutomationEngine when indoor data is ready
	QObject::connect(indoor_station, &IndoorStation::indoorDataReady, _automation_engine, &Automation::AutomationEngine::onIndoorStationData);
	QObject::connect(indoor_station, &IndoorStation::errorOccurred, _automation_engine, [this](const QString& error)
		{
			_automation_engine->onError(error);
		});
	QObject::connect(indoor_station, &IndoorStation::indoorDataReady, this, [this](const IndoorData& data)
		{
			ui->_indoor_data_l->setText(data.toString());
		});
	_indoor_station_thread->start();
}

void MainWindow::initAutomationEngine()
{
	_automation_engine = new Automation::AutomationEngine(_cfg.device_cfg_list, this);

	auto automation_widget = new Automation::AutomationWidget(_cfg.device_cfg_list, _automation_engine, this);
	ui->_manual_ctrl_layout->addWidget(automation_widget);

	_automation_engine->loadRules(_cfg.rules_cfg_relative_path);
}



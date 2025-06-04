#include "MainWindow.h"
#include "ErrorDetail.h"
#include "ForecastData.h"
#include "WeatherForecast.h"
#include "Logging.h"

#include <QtCore/QThread>

MainWindow::MainWindow(const Cfg::Config& cfg, QWidget* parent)
	: QMainWindow(parent), _cfg(cfg)
	, ui(new Ui::MainWindowClass())
{
	ui->setupUi(this);
}

MainWindow::~MainWindow()
{
	delete ui;

	// Clean up weatherforecast
	_weather_forecast_thread->exit();
	_weather_forecast_thread->wait();
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
			qDebug(main_win) << "Weather forecast ready:" << data.toString();
		});
	QObject::connect(forecast, &WFP::WeatherForecast::forecastDataReady, this, &MainWindow::onWeatherDataReady);

	QObject::connect(forecast, &WFP::WeatherForecast::errorOccurred, [](const ErrorDetail& error)
		{
			qDebug(main_win) << "Error in weather forecast:" << error.getErrorMessage();
		});

	_weather_forecast_thread->start();
}

void MainWindow::onWeatherDataReady(const WFP::ForecastData& data)
{
	ui->_test_l->setText(data.toString() + " Hellooo");
}


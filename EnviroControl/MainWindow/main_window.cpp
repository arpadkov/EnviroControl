#include "MainWindow.h"
#include "ErrorDetail.h"
#include "ForecastData.h"
#include "WeatherForecast.h"
#include "Logging.h"
#include "AutomationEngine.h"
#include "ManualDeviceControlWidget.h"

#include <QtCore/QThread>

MainWindow::MainWindow(const Cfg::Config& cfg, QWidget* parent)
	: QMainWindow(parent), _cfg(cfg)
	, ui(new Ui::MainWindowClass())
{
	ui->setupUi(this);

	initWeatherForecastThread();
	initAutomationEngine();
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
			qDebug(main_win_log) << "Weather forecast ready:" << data.toString();
		});
	QObject::connect(forecast, &WFP::WeatherForecast::forecastDataReady, this, &MainWindow::onWeatherDataReady);

	QObject::connect(forecast, &WFP::WeatherForecast::errorOccurred, [](const ErrorDetail& error)
		{
			qDebug(main_win_log) << "Error in weather forecast:" << error.getErrorMessage();
		});

	_weather_forecast_thread->start();
}

void MainWindow::initAutomationEngine()
{
	_automation_engine = new Automation::AutomationEngine(_cfg.device_cfg_list, this);

	_manual_device_control_widget = new Automation::ManualDeviceControlWidget(_cfg.device_cfg_list, this);
	ui->_manual_ctrl_layout->addWidget(_manual_device_control_widget);

	// Connect to widget signals
	connect(_manual_device_control_widget, &Automation::ManualDeviceControlWidget::deviceUpPressed, _automation_engine, &Automation::AutomationEngine::onManualDeviceUpRequest);
	connect(_manual_device_control_widget, &Automation::ManualDeviceControlWidget::deviceDownPressed, _automation_engine, &Automation::AutomationEngine::onManualDeviceDownRequest);
}

void MainWindow::onWeatherDataReady(const WFP::ForecastData& data)
{
	ui->_test_l->setText(data.toString() + " Hellooo");
}


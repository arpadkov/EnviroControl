#include "MainWindow.h"
#include "ErrorDetail.h"
#include "ForecastData.h"
#include "WeatherForecast.h"
#include "Logging.h"
#include "AutomationEngine.h"
#include "AutomationWidget.h"
#include "WeatherStation.h"
#include "WeatherStationMock.h"
#include "WeatherHistoryWidget.h"
#include "IndoorStationWidget.h"
#include "ErrorDetailsWidget.h"

#include <QtCore/QThread>
#include <QtCore/QFile>
#include <QtWidgets/QStyle>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QToolButton>

namespace
{
QString getStyleSheet(const QString& file_name)
{
	QFile style_file(QString(":/styles/style_sheets/%1.qss").arg(file_name));
	if (style_file.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream stream(&style_file);
		QString style_sheet = stream.readAll();
		style_file.close();
		return style_sheet;
	}
	return QString();
}
}

MainWindow::MainWindow(const Cfg::Config& cfg, QWidget* parent)
	: QMainWindow(parent), _cfg(cfg)
	, ui(new Ui::MainWindowClass())
{
	ui->setupUi(this);

	initNavigationBar();

	initErrorDisplyaWidget();

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
	ui->_weather_forecast_l->setText(data.toString());
}

void MainWindow::onNavButtonClicked()
{
	if (auto clicked_button = qobject_cast<QToolButton*>(sender()))
		activateNavButton(clicked_button);
}

void MainWindow::activateNavButton(QToolButton* btn)
{
	// Deactivate all buttons
	auto navbar_layout = ui->_navigation_bar->layout();
	if (!navbar_layout)
		return;

	for (int i = 0; i < navbar_layout->count(); ++i)
	{
		QWidget* widget = navbar_layout->itemAt(i)->widget();
		if (auto button = qobject_cast<QToolButton*>(widget))
		{
			button->style()->polish(button); // Repolish the style to remove the highlight
		}
	}

	// Activate the clicked button
	btn->setChecked(true);
	btn->style()->polish(btn);

	// Get the button's index and set the stacked widget
	int index = navbar_layout->indexOf(btn);
	ui->_main_stack->setCurrentIndex(index);
}

void MainWindow::initNavigationBar()
{
	auto navbar_btn_group = new QButtonGroup(this);
	navbar_btn_group->setExclusive(true);

	static const QSize NAVBAR_ICON_SIZE(64, 64);

	ui->_navigation_bar->setStyleSheet(getStyleSheet("navigation_bar"));
	ui->_navigation_bar->layout()->setContentsMargins(0, 0, 0, 0);
	ui->_navigation_bar->layout()->setSpacing(0);
	ui->_navigation_bar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setSpacing(0);

	// Connect navigation buttons to the main tab stack
	auto init_nav_btn = [this, navbar_btn_group](QToolButton* button, int index)
		{
			navbar_btn_group->addButton(button);
			button->setCheckable(true);
			button->setIcon(QIcon(":/main_tabs/icons/home.svg"));
			button->setIconSize(NAVBAR_ICON_SIZE);
			connect(button, &QToolButton::clicked, this, &MainWindow::onNavButtonClicked);
		};
	init_nav_btn(ui->_home_btn, 0);
	init_nav_btn(ui->_weather_history_btn, 1);
	init_nav_btn(ui->_weather_forecast_btn, 2);
	activateNavButton(ui->_home_btn); // Set the initial active button

	// Close button
	ui->_close_btn->setIcon(QIcon(":/main_tabs/icons/close.svg"));
	ui->_close_btn->setIconSize(NAVBAR_ICON_SIZE);
	connect(ui->_close_btn, &QToolButton::clicked, QApplication::instance(), &QApplication::quit);
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

	// Connect to widgets
	ui->_weather_station_widget->setStyleSheet(getStyleSheet("station_frame"));
	QObject::connect(weather_station, &IWeatherStation::errorOccurred, _error_details_widget, &ErrorDetailsWidget::onErrorOccurred);

	auto weather_history_widget = new WeatherHistoryWidget(this);
	ui->_weather_history_layout->addWidget(weather_history_widget);
	QObject::connect(weather_station, &IWeatherStation::weatherDataReady, weather_history_widget, &WeatherHistoryWidget::onWeatherData);
	QObject::connect(weather_station, &IWeatherStation::weatherDataReady, ui->_weather_station_widget, &WeatherStationWidget::onWeatherData);

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

	// Connect to widgets
	ui->_indoor_station_widget->setStyleSheet(ui->_indoor_station_widget->styleSheet() + getStyleSheet("station_frame"));
	QObject::connect(indoor_station, &IndoorStation::indoorDataReady, ui->_indoor_station_widget, &IndoorStationWidget::onIndoorDataChanged);

	QObject::connect(indoor_station, &IndoorStation::errorOccurred, _error_details_widget, &ErrorDetailsWidget::onErrorOccurred);

	_indoor_station_thread->start();
}

void MainWindow::initAutomationEngine()
{
	_automation_engine = new Automation::AutomationEngine(_cfg.device_cfg_list, this);

	auto automation_widget = new Automation::AutomationWidget(_cfg.device_cfg_list, _automation_engine, this);
	ui->_manual_ctrl_layout->addWidget(automation_widget);

	_automation_engine->loadRules(_cfg.rules_cfg_relative_path);
}

void MainWindow::initErrorDisplyaWidget()
{
	_error_details_widget = new ErrorDetailsWidget(this);
	ui->_error_display_l->addWidget(_error_details_widget);
}



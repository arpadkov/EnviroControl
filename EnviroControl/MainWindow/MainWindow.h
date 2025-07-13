#pragma once

#include "ConfigParser.h"

#include <QtWidgets/QMainWindow>
#include <QtCore/QPointer>
#include <ui_MainWindow.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindowClass;
};
QT_END_NAMESPACE

class QThread;

class WeatherData;

namespace WFP
{
class ForecastData;
}

namespace Automation
{
class AutomationEngine;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(const Cfg::Config& cfg, QWidget* parent = nullptr);
	~MainWindow();

public Q_SLOTS:
	void onWeatherForecastData(const WFP::ForecastData& data);
	void onWeatherData(const WeatherData& data);

private:
	void initWeatherForecastThread();
	void initWeatherStationThread();
	void initAutomationEngine();

private:
	Ui::MainWindowClass* ui;

	Cfg::Config _cfg;

	QThread* _weather_forecast_thread;
	QThread* _weather_station_thread;

	QPointer<Automation::AutomationEngine> _automation_engine;
};


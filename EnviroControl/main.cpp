#include "MainWindow.h"
#include "ConfigParser.h"
#include "ErrorDetail.h"
#include "Logging.h"
#include "WeatherForecast.h"
#include "ForecastData.h"

#include <QtCore/QThread>
#include <QtWidgets/QApplication>

WFP::WeatherForecast* createWeatherForecast()
{
	QString api_key = ""; // READ THIS FROM CONFIG FILE
	double latitude = 48.651810;
	double longitude = 10.300956;

	return new WFP::WeatherForecast({ latitude, longitude }, 600, nullptr);
}

int main(int argc, char* argv[])
{
	Log::init();

	const auto& cfg = ConfigParser::parseConfigFile();

	qDebug(appLog) << "Application started";

	QApplication app(argc, argv);
	MainWindow window;

	QThread weather_forecast_thread;
	auto forecast = createWeatherForecast();
	forecast->moveToThread(&weather_forecast_thread);

	QObject::connect(&weather_forecast_thread, &QThread::started, forecast, &WFP::WeatherForecast::startFetching);

	QObject::connect(forecast, &WFP::WeatherForecast::forecastDataReady, [](const WFP::ForecastData& data) {
		qDebug(appLog) << "Weather forecast ready:" << data.toString();
		});
	QObject::connect(forecast, &WFP::WeatherForecast::forecastDataReady, &window, &MainWindow::onWeatherDataReady);

	QObject::connect(forecast, &WFP::WeatherForecast::errorOccurred, [](const ErrorDetail& error) {
		qDebug(appLog) << "Error in weather forecast:" << error.getErrorMessage();
		});

	weather_forecast_thread.start();

	// RulesEngine (GUI thread)
	//  load/save rules from json file
	//  some widget has access to RulesEngine to add/edit/delete rules

	// AutomationEngine (GUI thread)
	//  Auto/Manual mode
	//  load rules from RulesEngine
	//  get data from Stations
	//  send desired states to DeviceStateManager
	//  rules are evaluated every minute -> task is sent to DeviceStateManager -> forgets about it
	//  rule evaluation does not happen here, just a call to RulesEngine::evaluateRules(WeatherData)
	//    -> returns a list of tasks
	//  internal timer to evaluate rules every minute

	// WeatherData
	//  struct to store indoor and outdoor weather data

	// IWeatherStation (seperate thread)
	//  emit signals on weatherDataRecived
	//  read from file on windows

	// IIndoorStation (seperate thread)
	//  emit signals on indoorDataRecived
	//  read from file on windows

	// DeviceStateManager (seperate thread)
	//  accept desired states from AutomationEngine
	//  translates states into tasks for IDeviceDriver
	//  send tasks to IDeviceDriver
	//  desired states are received every minute, but not always executed based on current state
	//  tasks are queried, and executed with timing 
	//   send task -> wait for 2 minutes to finish -> send next task
	//  tasks are logged, and current state is stored based on last tasks


	// IDeviceDriver (thread of DeviceStateManager)
	//  handle both window and sunblinds
	//  direct member ptr of DeviceStateManager
	//  does not care about state, just executes tasks (timing is handled by DeviceStateManager)
	//  different implementations for windows(log only) and pi
	//  maybe IWindowDriver and ISunblindDriver interfaces

	// WeatherForecast (seperate thread)
	//  OpenWeatherMap API
	//  Used only by WeatherForecastWidget
	//  internal timer, emit weatherForecastReady()


	// MainWindow (GUI thread)
	//  -> WeatherDisplayWidget
	//  -> IndoorDisplayWidget
	//  -> WeatherForecastWidget (kind of background)
	//  -> AutomationEngineWidget (expandable, auto/manual mode, controls for up/down)
	//  -> RulesWidget (expandable, add/edit/delete rules)
	//  -> DeviceStateManagerWidget (expandable (last sent cmd, current cmd etc))

	window.show();


	int result = app.exec();

	weather_forecast_thread.exit(); // Ensure the thread is stopped when the application exits
	weather_forecast_thread.wait(); // Wait for the thread to finish before exiting the application
	delete forecast;

	return result;
}

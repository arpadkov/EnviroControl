#include "MainWindow.h"
#include "ConfigParser.h"
#include "ErrorDetail.h"
#include "Logging.h"

#include <QtWidgets/QApplication>


int main(int argc, char* argv[])
{
	Log::init();

	auto cfg = Cfg::ConfigParser::parseConfigFile();
	if (!cfg)
	{
		qCritical(app_log) << "Could not read config file. Exiting";
		return 1;
	}

	qDebug(app_log) << "Application started";

	QApplication app(argc, argv);
	MainWindow window(*cfg);

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
	return app.exec();
}

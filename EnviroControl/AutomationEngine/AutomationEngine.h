#pragma once

#include <QtCore/QObject>

class WeatherData;

class DeviceStates;

namespace Automation
{
// AutomationEngine (GUI thread)
//  Auto/Manual mode
//  load rules from RulesEngine
//  get data from Stations
//  send desired states to DeviceStateManager (other thread)
//  rules are evaluated every minute -> task is sent to DeviceStateManager -> forgets about it
//  rule evaluation does not happen here, just a call to RulesEngine::evaluateRules(WeatherData)
//    -> returns a list of tasks
//  internal timer to evaluate rules every minute
class AutomationEngine : public QObject
{
	Q_OBJECT
public:
	explicit AutomationEngine(QObject* parent = nullptr);
	~AutomationEngine();

Q_SIGNALS:
	void deviceStatesCalculated(const DeviceStates& calulated_states);

public Q_SLOTS:
	void onWeatherStationData(const WeatherData& weather_data);
};
}
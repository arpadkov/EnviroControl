#pragma once

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QPointer>

class WeatherData;

namespace Device
{
class DeviceStates;
}

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
	~AutomationEngine() = default;

Q_SIGNALS:
	void deviceStatesUpdate(const Device::DeviceStates& calulated_states);

public Q_SLOTS:
	void onWeatherStationData(const WeatherData& weather_data);

private:
	void onCalcTimeout();

private:
	QPointer<QTimer> _calc_timer = nullptr;
	std::vector<WeatherData> _weather_data_history; // circular buffer, last is latest
	int _weather_data_history_length = 3600;
};
}
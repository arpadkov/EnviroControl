#pragma once

#include "ConfigParser.h"
#include "IndoorStation.h"
#include "RuleSet.h"
#include "WeatherData.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QPointer>


class QThread;

namespace Device
{
class DeviceStateManager;
class DeviceStates;
struct DeviceState;
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
	explicit AutomationEngine(const Cfg::DeviceConfigList& cfg, QObject* parent = nullptr);
	~AutomationEngine() = default;

	void loadRules(const QString& file_path);
	void setManualMode();
	void setAutoMode();

Q_SIGNALS:
	void deviceStatesUpdated(const Device::DeviceStates& calulated_states);
	void automationModeChanged(bool manual_mode);
	void manualDeviceRequest(const Device::DeviceState& state);

public Q_SLOTS:
	void onWeatherStationData(const WeatherData& weather_data);
	void onIndoorStationData(const IndoorData& indoor_data);
	void onManualDeviceUpRequest(const QString& device_id);
	void onManualDeviceDownRequest(const QString& device_id);

private:
	void onCalcTimeout();
	void initStateManagerThread();

private:
	QPointer<QTimer> _calc_timer = nullptr;
	std::vector<WeatherData> _weather_data_history;
	std::vector<IndoorData> _indoor_data_history;
	int _data_history_secs = 3600;
	Cfg::DeviceConfigList _devices_cfg;
	RuleSet _rule_set;

	// State manager thread
	QThread* _state_manager_thread = nullptr;
	Device::DeviceStateManager* _state_manager = nullptr;
	QMetaObject::Connection _automation_connect;
};
}
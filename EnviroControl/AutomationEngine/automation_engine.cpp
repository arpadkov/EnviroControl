#include "AutomationEngine.h"
#include "WeatherStation.h"
#include "DeviceStateManager.h"
#include "RulesProcessor.h"

#include <QtCore/QThread>

namespace Automation
{

namespace
{
void addCircularBufferData(std::vector<WeatherData>& buffer, const WeatherData& data, int max_size)
{
	buffer.insert(buffer.begin(), data);
	if (buffer.size() > max_size)
		buffer.pop_back();
}
}

AutomationEngine::AutomationEngine(const Cfg::DeviceConfigList& cfg, QObject* parent) :
	QObject(parent), _devices_cfg(cfg)
{
	_calc_timer = new QTimer(this);
	connect(_calc_timer, &QTimer::timeout, this, &AutomationEngine::onCalcTimeout);
	_calc_timer->start(5000);

	initStateManagerThread();

}

void AutomationEngine::setManualMode()
{
	_calc_timer->stop();
}

void AutomationEngine::setAutoMode()
{
	_calc_timer->start();
}

void AutomationEngine::onWeatherStationData(const WeatherData& weather_data)
{
	addCircularBufferData(_weather_data_history, weather_data, _weather_data_history_length);
}

void AutomationEngine::onManualDeviceUpRequest(const QString& device_id)
{
	setManualMode();

	Device::DeviceState state;
	state.device_id = device_id;
	state.position = Device::DevicePosition::Open;
	Q_EMIT manualDeviceRequest(state);
}

void AutomationEngine::onManualDeviceDownRequest(const QString& device_id)
{
	setManualMode();
	
	Device::DeviceState state;
	state.device_id = device_id;
	state.position = Device::DevicePosition::Closed;
	Q_EMIT manualDeviceRequest(state);
}

void AutomationEngine::onCalcTimeout()
{
	if (_weather_data_history.empty())
		return;

	const auto& calculated_states = RulesProcessor::calculateDeviceStates({}, _weather_data_history);
	Q_EMIT deviceStatesUpdated(calculated_states);
}

void AutomationEngine::initStateManagerThread()
{
	_state_manager_thread = new QThread();
	auto state_manager = new Device::DeviceStateManager(_devices_cfg);
	state_manager->moveToThread(_state_manager_thread);

	// Destruct on finished
	connect(_state_manager_thread, &QThread::finished, state_manager, &QObject::deleteLater);

	connect(this, &AutomationEngine::manualDeviceRequest,
		state_manager, &Device::DeviceStateManager::onManualDeviceRequest);

	_state_manager_thread->start();
}

}
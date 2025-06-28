#include "AutomationEngine.h"
#include "DeviceStateManager.h"
#include "RulesProcessor.h"

#include <QtCore/QThread>

namespace Automation
{

namespace
{
template<typename T>
void addCircularBufferData(std::vector<T>& buffer, const T& new_data, int max_size_seconds)
{
	buffer.insert(buffer.begin(), new_data);

	const QDateTime& newest_time = new_data.timestamp;
	while (!buffer.empty())
	{
		const QDateTime& oldest_time = buffer.back().timestamp;
		if (oldest_time.secsTo(newest_time) < max_size_seconds)
			break;

		buffer.pop_back();
	}
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

void AutomationEngine::loadRules(const QString& file_path)
{
	_rule_set.loadFromJson(file_path);
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
	addCircularBufferData(_weather_data_history, weather_data, _data_history_secs);
}

void AutomationEngine::onIndoorStationData(const IndoorData& indoor_data)
{
	addCircularBufferData(_indoor_data_history, indoor_data, _data_history_secs);
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

	std::vector<QString> device_ids;
	for (const auto& device_cfg : _devices_cfg.device_cfgs)
		device_ids.push_back(device_cfg.device_id);

	const auto& calculated_states = RulesProcessor::calculateDeviceStates(_rule_set, device_ids, _weather_data_history, _indoor_data_history);
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
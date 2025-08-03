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

AutomationEngine::~AutomationEngine()
{
	if (_calc_timer)
		_calc_timer->stop();
	if (_state_manager_thread)
	{
		_state_manager_thread->quit();
		_state_manager_thread->wait();
		delete _state_manager_thread;
	}
	qDebug() << "AutomationEngine: Destructor called, resources cleaned up.";
}

void AutomationEngine::loadRules(const QString& file_path)
{
	_rule_set.loadFromJson(file_path);
}

void AutomationEngine::setManualMode()
{
	qDebug() << "AutomationEngine: Switching to manual mode";
	disconnect(_automation_connect);
	Q_EMIT abortMovement();
}

void AutomationEngine::setAutoMode()
{
	qDebug() << "AutomationEngine: Switching to auto mode";

	// This connection only exists in auto mode, so we can safely disconnect it
	_automation_connect = connect(this, &AutomationEngine::deviceStatesUpdated,
		_state_manager, &Device::DeviceStateManager::onDeviceStatesUpdated);
}

void AutomationEngine::onWeatherStationData(const WeatherData& weather_data)
{
	addCircularBufferData(_weather_data_history, weather_data, _data_history_secs);
}

void AutomationEngine::onIndoorStationData(const IndoorData& indoor_data)
{
	addCircularBufferData(_indoor_data_history, indoor_data, _data_history_secs);
}

void AutomationEngine::onManualDeviceOpenRequest(const QString& device_id)
{
	setManualMode();

	Device::DeviceState state;
	state.device_id = device_id;
	state.position = Device::DevicePosition::Open;
	Q_EMIT manualDeviceRequest(state);
}

void AutomationEngine::onManualDeviceCloseRequest(const QString& device_id)
{
	setManualMode();

	Device::DeviceState state;
	state.device_id = device_id;
	state.position = Device::DevicePosition::Closed;
	Q_EMIT manualDeviceRequest(state);
}

void AutomationEngine::onAbort()
{
	setManualMode();
	Q_EMIT abortMovement();
}

void AutomationEngine::onError(const QString& error)
{
	setManualMode();
	_state_manager->onError();
}

void AutomationEngine::onCalcTimeout()
{
	if (_weather_data_history.empty() || _indoor_data_history.empty())
		return;

	std::vector<QString> device_ids;
	for (const auto& device_cfg : _devices_cfg.device_cfgs)
		device_ids.push_back(device_cfg.device_id);

	try
	{
		const auto& calculated_states = RulesProcessor::calculateDeviceStates(_rule_set, device_ids, _weather_data_history, _indoor_data_history);
		Q_EMIT deviceStatesUpdated(calculated_states);
	}
	catch (const std::runtime_error& e)
	{
		qCritical() << "AutomationEngine: Failed to calculate device states:" << e.what();
		return;
	}
}

void AutomationEngine::initStateManagerThread()
{
	_state_manager_thread = new QThread();
	_state_manager = new Device::DeviceStateManager(_devices_cfg);
	_state_manager->moveToThread(_state_manager_thread);

	// Destruct on finished
	connect(_state_manager_thread, &QThread::finished, _state_manager, &QObject::deleteLater);

	// Manual device request is always active
	connect(this, &AutomationEngine::manualDeviceRequest, _state_manager, &Device::DeviceStateManager::onManualDeviceRequest);
	connect(this, &AutomationEngine::abortMovement, _state_manager, &Device::DeviceStateManager::onAbort);

	_state_manager_thread->start();
}

}
#include "DeviceStateManager.h"
#include "DeviceDriver.h"

#include "Logging.h"

#include <QtCore/QTimer>

#include <algorithm>

namespace Device
{

DeviceStateManager::DeviceStateManager(const Cfg::DeviceConfigList& cfg, QObject* parent) :
	QObject(parent), _devices_cfg(cfg)
{
	registerDevices();

	// Configure reset timer
	_reset_timer = new QTimer(this);
	_reset_timer->setSingleShot(true);

	connect(this, &DeviceStateManager::deviceMovementFinished, this, &DeviceStateManager::calculateAndSetNextState);
}

DeviceStateManager::~DeviceStateManager()
{
}

void DeviceStateManager::onManualDeviceRequest(const Device::DeviceState& state)
{
	setDevicestate(state);
}

/*
* Store the desired device states, which are set by the AutomationEngine. Differences are calculated and set one after another.
* Called periodically, when AutomationEnginge updates the desired device states, based on the WeatherStation.
* MUST NOT be called if manual mode is activated
*/
void DeviceStateManager::onDeviceStatesUpdated(const Device::DeviceStates& states)
{
	_desired_states = states;
	
	if (!isAnyDeviceMoving())
		calculateAndSetNextState(); // Check if any device needs to be moved
}

void DeviceStateManager::onAbort()
{
	interruptCurrentMovement();
}

void DeviceStateManager::registerDevices()
{
	for (const auto& device_cfg : _devices_cfg.device_cfgs)
	{
		const auto& device_id = device_cfg.device_id;

		// Check if the device driver is already registered
		if (std::find_if(_device_drivers.begin(), _device_drivers.end(),
			[&device_id](const std::unique_ptr<IDeviceDriver>& driver)
			{
				return driver && driver->getId() == device_id;
			}) != _device_drivers.end())
		{
			qWarning(device_log) << "DeviceStateManager::registerDevices: Device with ID" << device_id << "is already registered.";
			continue;
		}

		std::unique_ptr<IDeviceDriver> driver;

		// TestDriver for Windows
#ifdef _WIN32
		driver = std::make_unique<TestDeviceDriver>(device_id, device_cfg.reset_time_sec);
#endif

		// Real driver for Raspberry Pi
#if defined(__linux__) && (defined(__ARM_ARCH) || defined(__arm__))
		driver = std::make_unique<DeviceDriver>(device_id, device_cfg.reset_time_sec, device_cfg.open_gpio_pin, device_cfg.close_gpio_pin, false);
#endif

		if (driver && driver->initialize())
		{
			qInfo(device_log) << "DeviceStateManager::registerDevices: Registering device with ID:" << device_id;
			_device_drivers.push_back(std::move(driver));

			// Initialize the device state for the new device with position Unknown
			Device::DeviceState initial_state;
			initial_state.device_id = device_id;
			initial_state.position = DevicePosition::Unknown;
			_device_states.states.push_back(initial_state);
		}
	}
}

IDeviceDriver* DeviceStateManager::getDeviceDriver(const QString& device_id)
{
	auto it = std::find_if(_device_drivers.begin(), _device_drivers.end(),
		[&device_id](const std::unique_ptr<IDeviceDriver>& driver)
		{
			if (!driver)
				return false;

			return driver->getId() == device_id;
		});

	return (it != _device_drivers.end()) ? it->get() : nullptr;
}

/*
* Sets the device state for a specific device. First reset all devices, and after timeout, also reset the sepcific device.
* If the device was not interrupted, updates the internal state of the specific device.
*/
void DeviceStateManager::setDevicestate(const Device::DeviceState& state)
{
	interruptCurrentMovement();

	const auto& device_id = state.device_id;
	auto device = getDeviceDriver(device_id);
	if (!device)
	{
		qCritical(device_log) << "DeviceStateManager::onManualDeviceRequest: Device driver not found for device ID:" << device_id;
		return;
	}

	// First set the device state internally to unkknown. Onces the timer times out, it will be set to the new state
	auto unknown_state = state;
	unknown_state.position = DevicePosition::Unknown;
	updateDevicestate(unknown_state);

	// Set the new state for the device
	switch (state.position)
	{
	case DevicePosition::Open:
		device->open();
		break;
	case DevicePosition::Closed:
		device->close();
		break;
	}

	// Notify external listeners that the device movement has started
	Q_EMIT deviceMovementStarted(state);

	_current_moving_device_id = device_id;

	// Start timout to reset the devices state after a certain time
	// Calling open() sends power to the drives, and the drives have internal limit switches.
	// But we want to avoid sending power indefinitely, so after the device has reached its position for sure, we reset the driver.
	connect(_reset_timer, &QTimer::timeout, this, [this, state]()
		{
			onResetTimerTimeout(state);
		}, Qt::SingleShotConnection); // After fired, delete the connection to avoid multiple calls
	_reset_timer->setInterval(device->getTimeoutSec() * 1000);
	_reset_timer->start();
}

void DeviceStateManager::onResetTimerTimeout(const Device::DeviceState& state)
{
	auto device = getDeviceDriver(state.device_id);
	if (device)
	{
		device->reset();

		// Also set the devices state internally, we know, that it has reached the position
		updateDevicestate(state);

		// Clear the current moving device ID
		_current_moving_device_id.clear();

		// Notify external listeners that the device movement has finished
		Q_EMIT deviceMovementFinished(state);
	}
	else
		qCritical(device_log) << "DeviceStateManager::setDevicestate: Device driver not found for ID: " << state.device_id;
}

/*
*	Update the internal state cache
*/
void DeviceStateManager::updateDevicestate(const Device::DeviceState& state)
{
	auto state_it = std::find_if(_device_states.states.begin(), _device_states.states.end(),
		[&state](const Device::DeviceState& s)
		{
			return s.device_id == state.device_id;
		});

	if (state_it != _device_states.states.end())
	{
		*state_it = state; // Update existing state
		qDebug(device_log) << "DeviceStateManager::updateDevicestate: Updated state for device ID:" << state.device_id << " - " << devicePositionToString(state.position);
	}
	else
		qCritical(device_log) << "DeviceStateManager::updateDevicestate: Device not found for ID: " << state.device_id;
}

/*
* Once a movement has been finished, check if there are any differences between desired and actual states.
* And execute the next movement if needed.
*/
void DeviceStateManager::calculateAndSetNextState()
{
	if (isAnyDeviceMoving())
	{
		qCritical(device_log) << "DeviceStateManager::calculateAndSetNextState: INTERNAL ERROR: some device is still moving";
		return;
	}
	qCritical(device_log) << "DeviceStateManager::calculateAndSetNextState: TEST - MOVEMENT FINISHED";

	for (const auto& new_state : _desired_states.states)
	{
		auto current_state_it = std::find_if(_device_states.states.begin(), _device_states.states.end(),
			[&new_state](const Device::DeviceState& s)
			{
				return s.device_id == new_state.device_id;
			});

		if (current_state_it == _device_states.states.end())
		{
			qCritical(device_log) << "DeviceStateManager::onDeviceStatesUpdated: Device with ID" << new_state.device_id << "not found in current states.";
			continue;
		}

		if (current_state_it->position == new_state.position)
			continue; // No change needed

		setDevicestate(new_state); // Start with the first device that has a different state
		return; // Only one device at a time for now
	}
}

bool DeviceStateManager::isAnyDeviceMoving() const
{
	return (!_reset_timer || _reset_timer->isActive() || !_current_moving_device_id.isEmpty());
}

/*
* Interrupt current operation:
*		stop running reset timer, because the drives will be resetted anyway
*		clean internal state of the current moving device
*   notify external listeners that the device movement was interrupted
*   (Unknown state is set, once the device starts moving -> not needed here)
*/
void DeviceStateManager::interruptCurrentMovement()
{
	// Reset all devices -> abort all movements (For safety, always start with this)
	for (const auto& driver : _device_drivers)
	{
		if (driver)
			driver->reset();
	}

	if (!_reset_timer)
	{
		qCritical(device_log) << "DeviceStateManager::interruptCurrentMovement: Reset timer is not initialized.";
		return;
	}

	// Nothing is running -> return
	if (_current_moving_device_id.isEmpty() && !_reset_timer->isActive())
		return;

	// Inconsistent state: Timer is running, but no device ID is set.
	if (_reset_timer->isActive() && _current_moving_device_id.isEmpty())
	{
		qCritical(device_log) << "DeviceStateManager::interruptCurrentMovement: INTERNAL ERROR: Reset timer is active but _current_moving_device_id is empty.";
		_reset_timer->stop();
		return;
	}

	// Inconsistent state: Device ID is set, but timer is NOT running.
	if (!_current_moving_device_id.isEmpty() && !_reset_timer->isActive())
	{
		qCritical(device_log) << "DeviceStateManager::interruptCurrentMovement: INTERNAL ERROR: _current_moving_device_id is set but reset timer is NOT active.";
		_current_moving_device_id.clear();
		return;
	}

	qDebug(device_log) << "DeviceStateManager::setDevicestate: Interrupting running reset timer.";
	_reset_timer->stop();
	_reset_timer->disconnect(); // Remove existing connections to avoid multiple calls

	_current_moving_device_id.clear(); // Clear the current moving device ID

	Q_EMIT deviceMovementInterrupted(_current_moving_device_id);
}

}
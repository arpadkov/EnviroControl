#pragma once

#include "ConfigParser.h"
#include "DeviceState.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMetaType>
#include <QtCore/QPointer>

class QTimer;

namespace Device
{

class IDeviceDriver;

inline QString devicePositionToString(DevicePosition pos)
{
	switch (pos)
	{
	case DevicePosition::Unknown: return "Unknown";
	case DevicePosition::Open:    return "Open";
	case DevicePosition::Closed:  return "Closed";
	default:                      return "Invalid"; // Handle unexpected values
	}
};

/*
* Holds the state of the devices, either the currently active state or the calculated state.
* Agnostic from how many devices there are, it should be read from the cfg file.
*/
class DeviceStates
{
public:
	DeviceStates()
	{
	};
	explicit DeviceStates(const std::vector<DeviceState>& states_) : states(states_)
	{
	};
	~DeviceStates() = default;

	std::optional<DevicePosition> getDevicePosition(const QString device_id) const
	{
		auto it = std::find_if(states.begin(), states.end(),
			[&device_id](const DeviceState& state)
			{
				return state.device_id == device_id;
			});

		if (it == states.end())
			return DevicePosition::Unknown;

		return it->position;
	};

	void setDevicePosition(const QString device_id, DevicePosition pos)
	{
		auto it = std::find_if(states.begin(), states.end(),
			[&device_id](const DeviceState& state)
			{
				return state.device_id == device_id;
			});

		if (it == states.end())
			return;

		it->position = pos;
	}

	std::vector<DeviceState> states;
};

// DeviceStateManager (seperate thread)
//  accept desired states from AutomationEngine
//  translates states into tasks for IDeviceDriver
//  send tasks to IDeviceDriver
//  desired states are received every minute, but not always executed based on current state
//  if there is a difference between desired state and current state, a task is sent to the driver
//   send task -> wait for 2 minutes to finish
// current state and current movement is stored based on last tasks
// on manual mode: current task is interrupted and new task is sent to the driver
// AutomationEngine must ensure, that no updated states are sent in manual mode

class DeviceStateManager : public QObject
{
	Q_OBJECT

public:
	DeviceStateManager(const Cfg::DeviceConfigList& cfg, QObject* parent = nullptr);
	~DeviceStateManager();

Q_SIGNALS:
	void deviceMovementStarted(const Device::DeviceState& state);
	void deviceMovementFinished(const Device::DeviceState& state);
	void deviceMovementInterrupted(const QString& device_id);

public Q_SLOTS:
	void onManualDeviceRequest(const Device::DeviceState& state);
	void onDeviceStatesUpdated(const Device::DeviceStates& state);
	void onAbort();
	void onError();

private:
	void registerDevices();
	IDeviceDriver* getDeviceDriver(const QString& device_id);
	void setDevicestate(const Device::DeviceState& state);
	void onResetTimerTimeout(const Device::DeviceState& state);
	void updateDevicestate(const Device::DeviceState& state);
	void calculateAndSetNextState();
	bool isAnyDeviceMoving() const;
	void interruptCurrentMovement();
	void removeDeviceDriver(const QString& device_id);

private:
	std::vector<std::unique_ptr<IDeviceDriver>> _device_drivers;
	Cfg::DeviceConfigList _devices_cfg;
	DeviceStates _device_states; // Last known states
	DeviceStates _desired_states;

	// Internal cache for current movement
	QPointer<QTimer> _reset_timer; // Only send signal to device, for a limited time
	QString _current_moving_device_id; // Device that is currently moving
};
}

Q_DECLARE_METATYPE(Device::DeviceState);
Q_DECLARE_METATYPE(Device::DeviceStates);
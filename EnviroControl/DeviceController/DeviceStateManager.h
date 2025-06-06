#pragma once

#include <QtCore/QString>

namespace Device
{

enum class DevicePosition : uint
{
	Open = 0,
	Closed = 1,
};

struct DeviceState
{
	QString device_id;
	DevicePosition position
};

/*
* Holds the state of the devices, either the currently active state or the calculated state.
* Agnostic from how many devices there are, it should be read from the cfg file.
*/
class DeviceStates
{
	// Dynamic list or map of DeviceState objects - with device names
};

// DeviceStateManager (seperate thread)
//  accept desired states from AutomationEngine
//  translates states into tasks for IDeviceDriver
//  send tasks to IDeviceDriver
//  desired states are received every minute, but not always executed based on current state
//  tasks are queried, and executed with timing 
//   send task -> wait for 2 minutes to finish -> send next task
//  tasks are logged, and current state is stored based on last tasks
// on manual mode: abort everything

class IDeviceStateManager
{

};
}
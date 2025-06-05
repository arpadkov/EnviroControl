#pragma once

namespace Device
{

// DeviceStateManager (seperate thread)
//  accept desired states from AutomationEngine
//  translates states into tasks for IDeviceDriver
//  send tasks to IDeviceDriver
//  desired states are received every minute, but not always executed based on current state
//  tasks are queried, and executed with timing 
//   send task -> wait for 2 minutes to finish -> send next task
//  tasks are logged, and current state is stored based on last tasks

class IDeviceStateManager
{

};
}
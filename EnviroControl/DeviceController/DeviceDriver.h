#pragma once

namespace Device
{

// IDeviceDriver (thread of DeviceStateManager)
//  handle both window and sunblinds
//  direct member ptr of DeviceStateManager
//  does not care about state, just executes tasks (timing is handled by DeviceStateManager)
//  different implementations for windows(log only) and pi
//  maybe IWindowDriver and ISunblindDriver interfaces

class IDeviceDriver
{ };

}
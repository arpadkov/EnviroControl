#pragma once

#include <QtCore/QString>

namespace Device
{

enum class DevicePosition : uint
{
	Unknown = 0,
	Open = 1,
	Closed = 2,
};

struct DeviceState
{
	QString device_id;
	DevicePosition position = DevicePosition::Unknown;
};

}
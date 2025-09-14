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

	QString deviceStateAsString(const QString device_id) const
	{
		auto pos = getDevicePosition(device_id).value_or(DevicePosition::Unknown);
		return devicePositionToString(pos);
	}

	std::vector<DeviceState> states;
};

}
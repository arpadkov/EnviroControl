#pragma once

#include "ConfigParser.h"
#include "DeviceState.h"

#include <QString>

#ifdef __linux__
#include <gpiod.hpp>
#endif

namespace Device
{

// IDeviceDriver (thread of DeviceStateManager)
//  handle both window and sunblinds (separata instances, that know about id and device type)
//  direct member ptr of DeviceStateManager
//  does not care about state, just executes tasks (timing is handled by DeviceStateManager)
//  different implementations for windows(log only) and pi
//  receives commands for start and stop, sends signal inbetween (no timing)

class IDeviceDriver
{
public:
	IDeviceDriver(const QString& id, int timeout_sec, DevicePosition safety_pos_) : _id(id), _timeout_sec(timeout_sec), safety_pos(safety_pos_)
	{
	};
	virtual ~IDeviceDriver() = default;

	virtual bool initialize() const = 0;
	virtual void open() const = 0;
	virtual void close() const = 0;
	virtual void reset() const = 0;

	QString getId() const
	{
		return _id;
	};

	int getTimeoutSec() const
	{
		return _timeout_sec;
	};

	DevicePosition safety_pos;

protected:
	const QString _id;
	const int _timeout_sec;
};

class TestDeviceDriver : public IDeviceDriver
{
public:
	TestDeviceDriver(const QString& id, int timeout_sec, DevicePosition safety_pos_);
	~TestDeviceDriver();

	bool initialize() const override;
	void open() const override;
	void close() const override;
	void reset() const override;
};

#ifdef __linux__
class DeviceDriver : public IDeviceDriver
{
public:
	DeviceDriver(const QString& id, int timeout_sec, DevicePosition safety_pos_, unsigned int open_gpio_line, unsigned int close_gpio_line, bool active_high);
	~DeviceDriver();

	bool initialize() const override;
	void open() const override;
	void close() const override;
	void reset() const override;

private:
	unsigned int _open_gpio_line;
	unsigned int _close_gpio_line;
	bool _active_high;

	// On Linux, store a pointer to the gpiod::line object
	// Note: gpiod::line is not copyable, so a pointer is safer for class members.
	mutable gpiod::line* _open_line; // mutable because initialize() changes it, but method is const
	mutable gpiod::line* _close_line; // mutable because initialize() changes it, but method is const

	inline int activeValue() const
	{
		return _active_high ? 1 : 0; // Value for ON state
	}

	inline int inactiveValue() const
	{
		return _active_high ? 0 : 1; // Value for OFF state
	}
};
#endif

} // namespace Device

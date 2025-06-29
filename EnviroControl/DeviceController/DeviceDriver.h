#pragma once

#include "ConfigParser.h"

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
	IDeviceDriver(const QString id) : _id(id)	{};
	virtual ~IDeviceDriver() = default;

	virtual bool initialize() const = 0;
	virtual void open() const = 0;
	virtual void close() const = 0;
	virtual void reset() const = 0;

	virtual QString getId() const = 0;

protected:
	QString _id;
};

class TestDeviceDriver : public IDeviceDriver
{
public:
	TestDeviceDriver(const QString& id);
	~TestDeviceDriver();

	bool initialize() const override;
	void open() const override;
	void close() const override;
	void reset() const override;

	QString getId() const override;
};

class DeviceDriver : public IDeviceDriver
{
public:
	DeviceDriver(const QString& id, unsigned int open_gpio_line, unsigned int close_gpio_line, bool active_high);
	~DeviceDriver();

	bool initialize() const override;
	void open() const override;
	void close() const override;
	void reset() const override;

	QString getId() const override;

private:
	unsigned int _open_gpio_line;
	unsigned int _close_gpio_line;
	bool _active_high;

#ifdef __linux__
	// On Linux, store a pointer to the gpiod::line object
	// Note: gpiod::line is not copyable, so a pointer is safer for class members.
	mutable gpiod::line* _open_line; // mutable because initialize() changes it, but method is const
	mutable gpiod::line* _close_line; // mutable because initialize() changes it, but method is const
#elif _WIN32
	// On Windows, we'll simulate the pin state
	mutable bool _simulated_open_value; // mutable because setValue() changes it
	mutable bool _simulated_close_value; // mutable because setValue() changes it
#endif
};

}
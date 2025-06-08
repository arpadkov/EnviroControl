#pragma once

#include "ConfigParser.h"

#include <QString>

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

	virtual void initialize() const = 0;
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

	void initialize() const override;
	void open() const override;
	void close() const override;
	void reset() const override;

	QString getId() const override;
};

}
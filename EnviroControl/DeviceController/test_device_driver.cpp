#include "DeviceDriver.h"

#include "Logging.h"


namespace Device
{
TestDeviceDriver::TestDeviceDriver(const QString& id) : IDeviceDriver(id)
{
	qDebug(device_log) << "TestDeviceDriver::CONSTRUCTOR() called with id:" << id;
}

TestDeviceDriver::~TestDeviceDriver()
{
	qDebug(device_log) << "TestDeviceDriver::DESTRUCTOR() called with id:" << _id;
}

void TestDeviceDriver::initialize() const
{
}

void TestDeviceDriver::open() const
{
	qDebug(device_log) << "TestDeviceDriver::open() called.";
}

void TestDeviceDriver::close() const
{
	qDebug(device_log) << "TestDeviceDriver::close() called.";
}

QString TestDeviceDriver::getId() const
{
	return _id;
}

}
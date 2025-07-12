#include "DeviceDriver.h"

#include "Logging.h"


namespace Device
{
TestDeviceDriver::TestDeviceDriver(const QString& id, int timeout_sec) : IDeviceDriver(id, timeout_sec)
{
	qDebug(device_log) << "TestDeviceDriver::CONSTRUCTOR() called with id:" << id;
}

TestDeviceDriver::~TestDeviceDriver()
{
	qDebug(device_log) << "TestDeviceDriver::DESTRUCTOR() called with id:" << _id;
}

bool TestDeviceDriver::initialize() const
{
	return true;
}

void TestDeviceDriver::open() const
{
	qDebug(device_log) << "TestDeviceDriver::open " << _id << " called.";
}

void TestDeviceDriver::close() const
{
	qDebug(device_log) << "TestDeviceDriver::close " << _id << " called.";
}

/*
* Stop all motions of the driver.
*/
void TestDeviceDriver::reset() const
{
	qDebug(device_log) << "TestDeviceDriver::reset " << _id << " called.";
}

}
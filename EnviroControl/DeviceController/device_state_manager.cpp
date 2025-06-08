#include "DeviceStateManager.h"
#include "DeviceDriver.h"

#include "Logging.h"

#include <algorithm>

namespace Device
{

DeviceStateManager::DeviceStateManager(const Cfg::DeviceConfigList& cfg, QObject* parent) :
	QObject(parent), _devices_cfg(cfg)
{
	registerDevices();
}

DeviceStateManager::~DeviceStateManager()
{
}

void DeviceStateManager::onManualDeviceRequest(const Device::DeviceState& state)
{
	auto device = getDeviceDriver(state.device_id);
	if (!device)
	{
		qCritical(device_log) << "DeviceStateManager::onManualDeviceRequest: Device driver not found for device ID:" << state.device_id;
		return;
	}

	for (const auto& driver : _device_drivers)
	{
		if (driver)
			driver->reset();
	}

	switch (state.position)
	{
	case DevicePosition::Open:
		device->open();
		break;
	case DevicePosition::Closed:
		device->close();
		break;
	}

}

void DeviceStateManager::registerDevices()
{
	for (const auto& device_cfg : _devices_cfg.device_cfgs)
	{
		const auto& device_id = device_cfg.device_id;

		// Check if the device driver is already registered
		if (std::find_if(_device_drivers.begin(), _device_drivers.end(),
			[&device_id](const std::unique_ptr<IDeviceDriver>& driver)
			{
				return driver && driver->getId() == device_id;
			}) != _device_drivers.end())
		{
			qWarning(device_log) << "DeviceStateManager::registerDevices: Device with ID" << device_id << "is already registered.";
			continue;
		}

		std::unique_ptr<IDeviceDriver> driver;
#ifdef _WIN32
		driver = std::make_unique<TestDeviceDriver>(device_id);
#endif

#if defined(__linux__) && (defined(__ARM_ARCH) || defined(__arm__))
		// TODO create actual DeviceDriver for pi
#endif

		if (driver)
		{
			qInfo(device_log) << "DeviceStateManager::registerDevices: Registering device with ID:" << device_id;
			_device_drivers.push_back(std::move(driver));
		}
	}
}

IDeviceDriver* DeviceStateManager::getDeviceDriver(const QString& device_id)
{
	auto it = std::find_if(_device_drivers.begin(), _device_drivers.end(),
		[&device_id](const std::unique_ptr<IDeviceDriver>& driver)
		{
			if (!driver)
				return false;

			return driver->getId() == device_id;
		});

	return (it != _device_drivers.end()) ? it->get() : nullptr;
}

}
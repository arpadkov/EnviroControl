#ifdef __linux__ // This class only has an implementation on linux

#include "DeviceDriver.h"

#include "Logging.h"

namespace Device
{

const QString GPIO_CHIP_NAME = "gpiochip0";

namespace
{

void gpioOperation(gpiod::line* line, int value, unsigned int line_number)
{
	if (line && line->is_requested())
	{
		try
		{
			line->set_value(value);
			qDebug() << "Linux: Set GPIO" << line_number << "to" << value;
		}
		catch (...)
		{
			qCritical() << "Linux: Error setting GPIO line" << line_number << ":";
		}
	}
	else
	{
		qWarning() << "Linux: GPIO" << line_number << "not initialized or requested. Cannot perform operation.";
	}
}

void inactivateAndRelaseLine(gpiod::line* line, int off_value, unsigned int line_number)
{
	if (line && line->is_requested())
	{
		try
		{
			line->set_value(off_value);
			line->release();
			qDebug() << "Relased GPIO line: " << line_number;
		}
		catch (...)
		{
			qCritical() << "Error releasing GPIO line" << line_number << ":";
		}
	}
	else
	{
		qWarning() << "GPIO" << line_number << "not initialized or requested. Cannot release.";
	}
	delete line;
}
}

DeviceDriver::DeviceDriver(const QString& device_id, int timeout_sec, DevicePosition safety_pos_, unsigned int open_gpio_line, unsigned int close_gpio_line, bool active_high) :
	IDeviceDriver(device_id, timeout_sec, safety_pos_), _open_gpio_line(open_gpio_line), _close_gpio_line(close_gpio_line), _active_high(active_high)
	, _open_line(nullptr)	, _close_line(nullptr)
{
}

DeviceDriver::~DeviceDriver()
{
	reset(); // Ensure GPIO lines are reset before destruction

	// Release lines and delete ptrs
	inactivateAndRelaseLine(_open_line, inactiveValue(), _open_gpio_line);
	inactivateAndRelaseLine(_close_line, inactiveValue(), _close_gpio_line);
}

bool DeviceDriver::initialize() const
{
	try
	{
		gpiod::chip chip(GPIO_CHIP_NAME.toStdString());
		qDebug() << "Linux: Opened GPIO chip:" << GPIO_CHIP_NAME;

		// Get the open GPIO line
		_open_line = new gpiod::line(chip.get_line(_open_gpio_line));
		_open_line->request(gpiod::line_request{
																		QString("qt-gpio-control-open-%1").arg(_id).toStdString(),
																		gpiod::line_request::DIRECTION_OUTPUT, // Correct enum for direction
																		0 // No specific flags needed
			},
			inactiveValue()); // Initial value (0 for OFF if active high, 1 for OFF if active low)
		qDebug() << "Linux: Successfully initialized Open GPIO line" << _open_gpio_line << "as output.";

		// Get the close GPIO line
		_close_line = new gpiod::line(chip.get_line(_close_gpio_line));
		_close_line->request(gpiod::line_request{
														 QString("qt-gpio-control-close-%1").arg(_id).toStdString(),
														 gpiod::line_request::DIRECTION_OUTPUT, // Correct enum for direction
														 0 // No specific flags needed
			},
			inactiveValue()); // Initial value (0 for OFF if active high, 1 for OFF if active low)
		qDebug() << "Linux: Successfully initialized Open GPIO line" << _close_gpio_line << "as output.";
	}
	catch (...)
	{
		qCritical() << "Linux: Failed to initialize GPIO line for device: " << _id << " : ";
		delete _open_line;
		delete _close_line;
		_open_line = nullptr;
		_close_line = nullptr;

		return false;
	}
	return true;
}

void DeviceDriver::open() const
{
	// Ensure close pin is OFF before activating open pin
	gpioOperation(_close_line, inactiveValue(), _close_gpio_line);
	gpioOperation(_open_line, activeValue(), _open_gpio_line);
}

void DeviceDriver::close() const
{
	// Ensure open pin is OFF before activating close pin
	gpioOperation(_open_line, inactiveValue(), _open_gpio_line);
	gpioOperation(_close_line, activeValue(), _close_gpio_line);
}

void DeviceDriver::reset() const
{
	gpioOperation(_open_line, inactiveValue(), _open_gpio_line);
	gpioOperation(_close_line, inactiveValue(), _close_gpio_line);
}

}
#endif
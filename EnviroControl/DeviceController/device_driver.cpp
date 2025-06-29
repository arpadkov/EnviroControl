#include "DeviceDriver.h"

#include "Logging.h"

namespace Device
{

// Define the GPIO chip name for Linux only
#ifdef __linux__
const QString GPIO_CHIP_NAME = "gpiochip0";
#endif

DeviceDriver::DeviceDriver(const QString& device_id, unsigned int open_gpio_line, unsigned int close_gpio_line, bool active_high) :
	IDeviceDriver(device_id), _open_gpio_line(open_gpio_line), _close_gpio_line(close_gpio_line), _active_high(active_high)
#ifdef __linux__
	, _open_line(nullptr) // Initialize gpiod::line pointer to nullptr on Linux
	, _close_line(nullptr) // Initialize gpiod::line pointer to nullptr on Linux
#elif _WIN32
	, _simulated_open_value(!active_high) // Initialize simulated value to OFF state on Windows
	, _simulated_close_value(!active_high) // Initialize simulated value to OFF state on Windows
#endif
{
}

DeviceDriver::~DeviceDriver()
{
#ifdef __linux__
	// Release the Open GPIO line when the object is destroyed on Linux
	if (_open_line && _open_line->is_requested())
	{
		try
		{
			// Set to default (off) state before releasing
			// If active high, set low (0). If active low, set high (1).
			_open_line->set_value(_active_high ? 0 : 1);
			_open_line->release();
			qDebug() << "Linux: Released GPIO line" << _open_gpio_line;
		}
		catch (...)
		{
			qCritical() << "Linux: Error releasing GPIO line" << _open_gpio_line << ":";
		}
	}
	delete _open_line; // Clean up the dynamically allocated gpiod::line object

	// Release the Close GPIO line when the object is destroyed on Linux
	if (_close_line && _close_line->is_requested())
	{
		try
		{
			// Set to default (off) state before releasing
			// If active high, set low (0). If active low, set high (1).
			_close_line->set_value(_active_high ? 0 : 1);
			_close_line->release();
			qDebug() << "Linux: Released GPIO line" << _close_gpio_line;
		}
		catch (...)
		{
			qCritical() << "Linux: Error releasing GPIO line" << _close_gpio_line << ":";
		}
	}
	delete _close_line; // Clean up the dynamically allocated gpiod::line object
#endif
	// No specific cleanup needed for Windows mock
}

bool DeviceDriver::initialize() const
{
#ifdef __linux__
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
			_active_high ? 0 : 1); // Initial value (0 for OFF if active high, 1 for OFF if active low)
		qDebug() << "Linux: Successfully initialized Open GPIO line" << _open_gpio_line << "as output.";

		// Get the close GPIO line
		_close_line = new gpiod::line(chip.get_line(_close_gpio_line));
		_close_line->request(gpiod::line_request{
														 QString("qt-gpio-control-close-%1").arg(_id).toStdString(),
														 gpiod::line_request::DIRECTION_OUTPUT, // Correct enum for direction
														 0 // No specific flags needed
			},
			_active_high ? 0 : 1); // Initial value (0 for OFF if active high, 1 for OFF if active low)
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
#elif _WIN32
	_simulated_open_value = !_active_high; // Ensure mock is in OFF state on init
	_simulated_close_value = !_active_high; // Ensure mock is in OFF state on init
	qDebug() << "Windows: Mock GPIO line" << _open_gpio_line << " & " << _close_gpio_line << "initialized. (No actual hardware interaction)";
	return true;
#else
	qWarning() << "Unsupported operating system for GPIO control.";
	return false;
#endif
}

void DeviceDriver::open() const
{
	qDebug() << "DeviceDriver" << _id << ": Opening device on GPIO" << _open_gpio_line << "...";
#ifdef __linux__
	if (_open_line && _open_line->is_requested() && _close_line && _close_line->is_requested())
	{
		try
		{
			// Ensure close pin is OFF before activating open pin
			_close_line->set_value(_active_high ? 0 : 1); // Set to OFF
			// Set value to activate relay: 1 for active-HIGH ON, 0 for active-LOW ON
			_open_line->set_value(_active_high ? 1 : 0);
			qDebug() << "Linux: Set GPIO" << _open_gpio_line << "to" << (_active_high ? "HIGH (ON)" : "LOW (ON)");
		}
		catch (...)
		{
			qCritical() << "Linux: Failed to turn ON relay on GPIO" << _open_gpio_line << ":";
		}
	}
	else
	{
		qWarning() << "Linux: GPIO" << _open_gpio_line << "not initialized or requested. Cannot open.";
	}
#elif _WIN32
	_simulated_open_value = _active_high; // Simulate ON state for open
	_simulated_close_value = !_active_high; // Simulate OFF state for close
	qDebug() << "Windows: Mock GPIO" << _open_gpio_line << "set to" << (_active_high ? "HIGH (ON)" : "LOW (ON)")
		<< ", Mock GPIO" << _close_gpio_line << "set to" << (_active_high ? "LOW (OFF)" : "HIGH (OFF)")
		<< "(Simulated values)";
#else
	qWarning() << "Unsupported operating system for GPIO control. Cannot open.";
#endif
}

void DeviceDriver::close() const
{
	qDebug() << "DeviceDriver" << _id << ": Closing device on GPIO" << _close_gpio_line << "...";
#ifdef __linux__
	if (_close_line && _close_line->is_requested() && _open_line && _open_line->is_requested())
	{
		try
		{
			// Ensure open pin is OFF before activating close pin
			_open_line->set_value(_active_high ? 0 : 1); // Set to OFF
			// Set value to deactivate relay: 0 for active-HIGH OFF, 1 for active-LOW OFF
			_close_line->set_value(_active_high ? 1 : 0);
			qDebug() << "Linux: Set GPIO" << _close_gpio_line << "to" << (_active_high ? "LOW (OFF)" : "HIGH (OFF)");
		}
		catch (...)
		{
			qCritical() << "Linux: Failed to turn OFF relay on GPIO" << _close_gpio_line << ":";
		}
	}
	else
	{
		qWarning() << "Linux: GPIO" << _close_gpio_line << "not initialized or requested. Cannot close.";
	}
#elif _WIN32
	_simulated_open_value = _active_high; // Simulate ON state for close
	_simulated_close_value = !_active_high; // Simulate OFF state for open
	qDebug() << "Windows: Mock GPIO" << _close_gpio_line << "set to" << (_active_high ? "HIGH (ON)" : "LOW (ON)")
		<< ", Mock GPIO" << _open_gpio_line << "set to" << (_active_high ? "LOW (OFF)" : "HIGH (OFF)")
		<< "(Simulated values)";
#else
	qWarning() << "Unsupported operating system for GPIO control. Cannot close.";
#endif
}

void DeviceDriver::reset() const
{
	qDebug() << "DeviceDriver" << _id << ": Resetting device on GPIOs" << _open_gpio_line << "and" << _close_gpio_line << "...";
#ifdef __linux__
	if (_open_line && _open_line->is_requested() && _close_line && _close_line->is_requested())
	{
		try
		{
			// Explicitly set both pins to their unpowered (OFF) state
			_open_line->set_value(_active_high ? 0 : 1); // Set Open pin OFF
			_close_line->set_value(_active_high ? 0 : 1); // Set Close pin OFF
			qDebug() << "Linux: Both GPIOs" << _open_gpio_line << "and" << _close_gpio_line << "set to OFF state.";
		}
		catch (...)
		{
			qCritical() << "Linux: Failed to reset device on GPIOs" << _open_gpio_line << "and" << _close_gpio_line << ":";
		}
	}
	else
	{
		qWarning() << "Linux: GPIO lines for" << _id << "not initialized or requested. Cannot reset.";
	}
#elif _WIN32
	_simulated_open_value = !_active_high;  // Simulate OFF state for open
	_simulated_close_value = !_active_high; // Simulate OFF state for close
	qDebug() << "Windows: Mock GPIOs" << _open_gpio_line << "and" << _close_gpio_line << "set to OFF state.";
#else
	qWarning() << "Unsupported operating system for GPIO control. Cannot reset.";
#endif
}

QString DeviceDriver::getId() const
{
	return _id;
}

}
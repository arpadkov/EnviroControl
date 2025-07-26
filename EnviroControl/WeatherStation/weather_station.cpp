#include "WeatherStation.h"
#include "Logging.h"

#include "ConfigParser.h"

#include <QtCore/QTimeZone>

const int PACKET_LENGHT = 40;
const char START_IDENTIFIER = 'W'; // Start of Weather Data 
const unsigned char END_IDENTIFIER = 0x03; // End identifier 0x03 

// Byte offsets for various data fields within the 40-byte packet 
const int TEMP_SIGN_OFFSET = 1;
const int TEMP_DIGIT1_OFFSET = 2;
const int TEMP_DIGIT2_OFFSET = 3;
const int TEMP_DECIMAL_OFFSET = 4; // This is where the decimal point is implied, not a character
const int TEMP_DIGIT3_OFFSET = 5;

const int SUN_SOUTH_DIGIT1_OFFSET = 6;
const int SUN_SOUTH_DIGIT2_OFFSET = 7;
const int SUN_WEST_DIGIT1_OFFSET = 8;
const int SUN_WEST_DIGIT2_OFFSET = 9;
const int SUN_EAST_DIGIT1_OFFSET = 10;
const int SUN_EAST_DIGIT2_OFFSET = 11;

const int TWILIGHT_OFFSET = 12;

const int DAYLIGHT_DIGIT1_OFFSET = 13;
const int DAYLIGHT_DIGIT2_OFFSET = 14;
const int DAYLIGHT_DIGIT3_OFFSET = 15;

const int WIND_DIGIT1_OFFSET = 16;
const int WIND_DIGIT2_OFFSET = 17;
const int WIND_DECIMAL_OFFSET = 18; // Implied decimal point
const int WIND_DIGIT3_OFFSET = 19;

const int RAIN_OFFSET = 20;

// Checksum offsets 
const int CHECKSUM_DIGIT1_OFFSET = 35; // Thousands digit
const int CHECKSUM_DIGIT2_OFFSET = 36; // Hundreds digit
const int CHECKSUM_DIGIT3_OFFSET = 37; // Tens digit
const int CHECKSUM_DIGIT4_OFFSET = 38; // Units digit
const int CHECKSUM_CALC_MAX_BYTE = 35; // Checksum is calculated up to byte 35 

WeatherStation::WeatherStation(const Cfg::WeatherStationConfig& cfg, QObject* parent) :
	QObject(parent), _cfg(cfg), _data_logger(cfg.log_file_path, cfg.log_frequency_sec, this)
{
	connect(this, &WeatherStation::weatherDataReady, &_data_logger, &WeatherDataLogger::onWeatherDataReady);
}

WeatherStation::~WeatherStation()
{
	stopReading();
}

void WeatherStation::startReading()
{
	initSerialPort();

	if (_port->portName().isEmpty())
	{
		qWarning() << "WeatherStation: Port name is empty. Cannot start reading.";
		Q_EMIT errorOccurred("Port name is empty. Cannot start reading.");
		return;
	}

	if (!_port->open(QIODevice::ReadOnly))
	{
		qWarning() << "WeatherStation: Failed to open port " << _port->portName() << " error:" << _port->errorString();
		Q_EMIT errorOccurred(QString("Failed to open port %1: %2").arg(_port->portName(), _port->errorString()));
		return;
	}

	qDebug() << "WeatherStation: Port " << _port->portName() << " opened successfully.";
}

void WeatherStation::stopReading()
{
	if (_port->isOpen())
	{
		_port->close();
		qDebug() << "WeatherStation: Port " << _port->portName() << " closed.";
	}
	else
	{
		qWarning() << "WeatherStation: Port " << _port->portName() << " is not open.";
	}
}

void WeatherStation::initSerialPort()
{
	_port = new QSerialPort(this);

	_port->setPortName(_cfg.port_name);
	_port->setBaudRate(_cfg.baud_rate);
	_port->setDataBits(QSerialPort::DataBits(_cfg.data_bits));
	_port->setStopBits(QSerialPort::StopBits(_cfg.stop_bits));
	_port->setParity(QSerialPort::Parity(_cfg.parity));

	connect(_port, &QSerialPort::readyRead, this, &WeatherStation::handleReadyRead);
}

void WeatherStation::handleReadyRead()
{
	_read_buffer.append(_port->readAll());

	// Loop to process all complete packets found in the buffer
	while (true)
	{
		// 1. Find the start of a potential packet ('W')
		int start_index = _read_buffer.indexOf(START_IDENTIFIER);

		// 2. If the START_IDENTIFIER is not found at all:
		//    Clear the buffer if it's getting too large with junk data to prevent memory issues.
		if (start_index == -1)
		{
			if (_read_buffer.size() > (PACKET_LENGHT * 2)) // Example: clear if buffer is twice the packet length and no start found
			{
				qWarning() << "WeatherStation: Start identifier not found in buffer. Discarding old data to prevent overflow.";
				_read_buffer.clear();
			}
			break; // No packet start found, wait for more data
		}

		// 3. If the START_IDENTIFIER is found, but there's leading junk:
		//    Discard the junk bytes before the actual start of the packet.
		if (start_index > 0)
		{
			qDebug() << "WeatherStation: Discarding " << start_index << " leading junk bytes before packet start.";
			_read_buffer.remove(0, start_index);
			// After this, 'W' is now at index 0 of _read_buffer
		}

		// 4. Now that 'W' is at index 0, check if there's enough data for a full packet.
		if (_read_buffer.size() < PACKET_LENGHT)
		{
			break; // Not enough data for a full packet yet, wait for more
		}

		// 5. Extract the complete, properly aligned packet.
		//    Since 'W' is at index 0, take exactly PACKET_LENGHT bytes from the beginning.
		QByteArray current_packet = _read_buffer.left(PACKET_LENGHT);

		// 6. Remove the processed packet from the buffer.
		_read_buffer.remove(0, PACKET_LENGHT);

		// 7. Attempt to parse the packet
		if (auto weather_data = parseWeatherData(current_packet))
		{
			Q_EMIT weatherDataReady(weather_data.value());
			// Packet successfully parsed, continue loop to check for next packet in buffer
		}
		else
		{
			// Parsing failed (e.g., checksum mismatch, or internal error in parseWeatherData).
			// The problematic packet has already been removed.
			qWarning() << "WeatherStation: Failed to parse weather data from packet: " << current_packet.toHex();
			// Do NOT break here, continue the loop to try and find the next valid packet in case of a corrupted one.
		}
	}
}

std::optional<WeatherData> WeatherStation::parseWeatherData(QByteArray data)
{
	if (data.size() < PACKET_LENGHT)
	{
		qWarning() << "WeatherStation: Data packet too short: " << data.size();
		Q_EMIT errorOccurred("Data packet too short");
		return {};
	}

	// Verify start & end signatures
	if (data.at(0) != START_IDENTIFIER)
	{
		qWarning() << "Invalid start identifier in packet.";
		Q_EMIT errorOccurred("Invalid start identifier in packet");
		return {};
	}

	if (static_cast<unsigned char>(data.at(PACKET_LENGHT - 1)) != END_IDENTIFIER)
	{
		qWarning() << "Invalid end identifier in packet.";
		Q_EMIT errorOccurred("Invalid end identifier in packet");
		return {};
	}

	if (!compareChecksum(data))
	{
		qWarning() << "Checksum mismatch in packet: " << data;
		Q_EMIT errorOccurred("Checksum mismatch in packet");
		return {};
	}

	WeatherData weather_data;

	// Temperature parsing
	QString temp_sign = (data.at(TEMP_SIGN_OFFSET) == '-') ? "-" : "";
	QString temp_str = temp_sign + QString(data.at(TEMP_DIGIT1_OFFSET)) +
		QString(data.at(TEMP_DIGIT2_OFFSET)) +
		QString(data.at(TEMP_DECIMAL_OFFSET)) + // This is the '.' character
		QString(data.at(TEMP_DIGIT3_OFFSET));
	weather_data.temperature = temp_str.toDouble();

	// Sun intensity parsing
	weather_data.sun_south = (QString(data.at(SUN_SOUTH_DIGIT1_OFFSET)) + QString(data.at(SUN_SOUTH_DIGIT2_OFFSET))).toDouble();
	weather_data.sun_west = (QString(data.at(SUN_WEST_DIGIT1_OFFSET)) + QString(data.at(SUN_WEST_DIGIT2_OFFSET))).toDouble();
	weather_data.sun_east = (QString(data.at(SUN_EAST_DIGIT1_OFFSET)) + QString(data.at(SUN_EAST_DIGIT2_OFFSET))).toDouble();

	// Twiglight parsing
	weather_data.twighlight = (data.at(TWILIGHT_OFFSET) == 'J');

	// Daylight parsing
	weather_data.daylight = (QString(data.at(DAYLIGHT_DIGIT1_OFFSET)).toInt() * 100) +
		(QString(data.at(DAYLIGHT_DIGIT2_OFFSET)).toInt() * 10) +
		QString(data.at(DAYLIGHT_DIGIT3_OFFSET)).toInt();

	// Wind parsing
	QString wind_str = QString(data.at(WIND_DIGIT1_OFFSET)) +
		QString(data.at(WIND_DIGIT2_OFFSET)) +
		"." + QString(data.at(WIND_DIGIT3_OFFSET));
	weather_data.wind = wind_str.toDouble();

	// Rain parsing
	weather_data.rain = (data.at(RAIN_OFFSET) == 'J');

	// Add current timestamp
	weather_data.timestamp = QDateTime::currentDateTime();

	return weather_data;
}

bool WeatherStation::compareChecksum(const QByteArray& data) const
{
	unsigned int received_checksum = (data.mid(CHECKSUM_DIGIT1_OFFSET, 1).toInt() * 1000) +
		(data.mid(CHECKSUM_DIGIT2_OFFSET, 1).toInt() * 100) +
		(data.mid(CHECKSUM_DIGIT3_OFFSET, 1).toInt() * 10) +
		(data.mid(CHECKSUM_DIGIT4_OFFSET, 1).toInt());

	unsigned int calculated_checksum = 0;
	for (int i = 0; i < CHECKSUM_CALC_MAX_BYTE; ++i)
	{
		calculated_checksum += static_cast<unsigned char>(data.at(i));
	}

	return received_checksum == calculated_checksum;
}

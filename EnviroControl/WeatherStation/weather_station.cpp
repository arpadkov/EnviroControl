#include "WeatherStation.h"
#include "Logging.h"

#include "ConfigParser.h"

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

WeatherStation::WeatherStation(const Cfg::WeatherStationConfig& cfg, QObject* parent) : QObject(parent)
{
	configurePort(cfg);

	connect(&_port, &QSerialPort::readyRead, this, &WeatherStation::handleReadyRead);
}

WeatherStation::~WeatherStation()
{
	stopReading();
}

void WeatherStation::startReading()
{
	if (_port.portName().isEmpty())
	{
		qWarning() << "WeatherStation: Port name is empty. Cannot start reading.";
		Q_EMIT errorOccurred("Port name is empty. Cannot start reading.");
		return;
	}

	if (!_port.open(QIODevice::ReadOnly))
	{
		qWarning() << "WeatherStation: Failed to open port " << _port.portName() << " error:" << _port.errorString();
		Q_EMIT errorOccurred(QString("Failed to open port %1: %2").arg(_port.portName(), _port.errorString()));
		return;
	}

	qDebug() << "WeatherStation: Port " << _port.portName() << " opened successfully.";
}

void WeatherStation::stopReading()
{
	if (_port.isOpen())
	{
		_port.close();
		qDebug() << "WeatherStation: Port " << _port.portName() << " closed.";
	}
	else
	{
		qWarning() << "WeatherStation: Port " << _port.portName() << " is not open.";
	}
}

void WeatherStation::configurePort(const Cfg::WeatherStationConfig& cfg)
{
	_port.setPortName(cfg.port_name);
	_port.setBaudRate(cfg.baud_rate);
	_port.setDataBits(QSerialPort::DataBits(cfg.data_bits));
	_port.setStopBits(QSerialPort::StopBits(cfg.stop_bits));
	_port.setParity(QSerialPort::Parity(cfg.parity));
}

void WeatherStation::handleReadyRead()
{
	_read_buffer.append(_port.readAll());

	while (_read_buffer.size() >= PACKET_LENGHT)
	{
		QByteArray current_packet = _read_buffer.left(PACKET_LENGHT);
		_read_buffer.remove(0, PACKET_LENGHT);

		if (auto weather_data = parseWeatherData(current_packet))
		{
			Q_EMIT weatherDataReady(weather_data.value());
		}
		else
		{
			qWarning() << "WeatherStation: Failed to parse weather data from packet: " << current_packet;
		}
	}
}

std::optional<WeatherData> WeatherStation::parseWeatherData(const QByteArray& data) const
{
	if (data.size() < PACKET_LENGHT)
	{
		qWarning() << "WeatherStation: Data packet too short: " << data.size();
		return {};
	}

	// Verify start & end signatures
	if (data.at(0) != START_IDENTIFIER)
	{
		qWarning() << "Invalid start identifier in packet.";
		return {};
	}

	if (static_cast<unsigned char>(data.at(PACKET_LENGHT - 1)) != END_IDENTIFIER)
	{
		qWarning() << "Invalid end identifier in packet.";
		return {};
	}

	if (!compareChecksum(data))
	{
		qWarning() << "Checksum mismatch in packet: " << data;
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

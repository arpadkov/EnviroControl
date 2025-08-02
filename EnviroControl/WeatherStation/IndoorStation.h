#pragma once

#include "ConfigParser.h"

#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtCore/QPointer>

class QProcess;

struct IndoorData
{
	double temperature = 0.0; // in Celsius
	double humidity = 0.0;    // in percentage
	QDateTime timestamp;

	QString toString() const
	{
		return QString("Temperature: %1 C\nHumidity: %2 %\nTimestamp: %3")
			.arg(temperature)
			.arg(humidity)
			.arg(timestamp.toString(Qt::ISODate));
	};
};

class IndoorStation : public QObject
{
	Q_OBJECT

public:
	IndoorStation(const Cfg::IndoorStationConfig& cfg, QObject* parent = nullptr);

public Q_SLOTS:
	void startReading();
	void stopReading();

Q_SIGNALS:
	void indoorDataReady(const IndoorData& data);
	void errorOccurred(const QString& error);

private:
	void initPyDHT22Process();
	std::optional<IndoorData> parseData(const QString& data_str) const;

	Cfg::IndoorStationConfig _cfg;
	QPointer<QProcess> _dht_reader_process; // Pointer to the Python process for reading data
};
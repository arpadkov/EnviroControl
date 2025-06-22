#pragma once

#include <QtCore/QObject>
#include <QtCore/QDateTime>

struct IndoorData
{
	double temperature = 0.0; // in Celsius
	double humidity = 0.0;    // in percentage
	QDateTime timestamp;
};

class IndoorStation : public QObject
{
	Q_OBJECT

public:
	IndoorStation(QObject* parent = nullptr);

};
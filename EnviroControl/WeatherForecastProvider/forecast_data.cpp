#include "ForecastData.h"

#include <QJsonObject>
#include <QJsonDocument>

namespace WFP
{
ForecastData::ForecastData(QJsonObject json_object)
{
	if (json_object.contains("main") && json_object["main"].isObject())
	{
		QJsonObject main_object = json_object["main"].toObject();

		if (main_object.contains("temp") && main_object["temp"].isDouble())
		{
			_temperature = main_object["temp"].toDouble();
			return;
		}
		else
		{
			qDebug() << "Error: 'temp' key not found or not a double in 'main' object.";
			return;
		}
	}
	else
	{
		qDebug() << "Error: 'main' object not found in the weather data.";
		return;
	}
}

QString ForecastData::toString() const
{
	return QString::number(_temperature) + " C";
}

}
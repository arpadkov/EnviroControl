#pragma once

#include <QMetaType>

class QJsonObject;
class QString;

namespace WFP
{

class ForecastData
{
public:
	ForecastData(QJsonObject json_object);

	QString toString() const;

private:
	double _temperature = 0.0;
};

}

Q_DECLARE_METATYPE(WFP::ForecastData);
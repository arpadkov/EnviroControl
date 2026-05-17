#include "WeatherHistoryWidgetBase.h"

#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QChart>
#include <QtCharts/QXYSeries>

WeatherHistoryWidgetBase::WeatherHistoryWidgetBase(int history_length_sec, QWidget* parent)
	: QWidget(parent), _history_length_sec(history_length_sec)
{
}

WeatherHistoryWidgetBase::~WeatherHistoryWidgetBase()
{
}

void WeatherHistoryWidgetBase::onWeatherData(const WeatherData& data)
{
	_weather_history.push_back(data);

	// Limit history to the specified time duration length
	QDateTime oldest_to_keep = data.timestamp.addSecs(-_history_length_sec);
	while (!_weather_history.empty() && _weather_history.front().timestamp < oldest_to_keep)
		_weather_history.erase(_weather_history.begin());

	updateCharts();
}

void WeatherHistoryWidgetBase::adjustXAxisRange(QChart* chart, const std::vector<WeatherData>& history)
{
	auto x_axis = qobject_cast<QDateTimeAxis*>(chart->axes(Qt::Horizontal).at(0));
	if (x_axis)
	{
		if (!history.empty())
		{
			x_axis->setRange(history.front().timestamp, history.back().timestamp);
		}
		else
		{
			x_axis->setRange(QDateTime::currentDateTime(), QDateTime::currentDateTime());
		}
	}
}

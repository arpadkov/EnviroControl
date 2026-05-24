#include "WeatherHistoryWidgetBase.h"

#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QXYSeries>

static const int DEFAULT_HISTORY_LENGTH_SEC = 3600; // 1 hour
static const int DEFAULT_DISPLAY_LENGTH_SEC = 1800; // 30 minutes

WeatherHistoryWidgetBase::WeatherHistoryWidgetBase(QWidget* parent)
	: QWidget(parent), _history_length_sec(DEFAULT_HISTORY_LENGTH_SEC), _display_length_sec(DEFAULT_DISPLAY_LENGTH_SEC),
	_chart(new QChart()), _chart_view(new QChartView(_chart, this))
{
	_weather_history = std::make_shared<std::vector<WeatherData>>();

	_chart->setMargins(QMargins(0, 0, 0, 0)); // Set all margins to 0
	_chart->setContentsMargins(0, 0, 0, 0);  // Set content margins to 0
}

WeatherHistoryWidgetBase::~WeatherHistoryWidgetBase()
{}

void WeatherHistoryWidgetBase::onWeatherData(const WeatherData& data)
{
	_weather_history->push_back(data);

	// Limit history to the specified time duration length
	QDateTime oldest_to_keep = data.timestamp.addSecs(-_history_length_sec);
	while (!_weather_history->empty() && _weather_history->front().timestamp < oldest_to_keep)
		_weather_history->erase(_weather_history->begin());

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

#include "WeatherHistoryWidget.h"
#include "WindRainChartWidget.h"
#include "SunChartWidget.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTabWidget>

static const int history_length_sec = 900; // 15 minutes

WeatherHistoryWidget::WeatherHistoryWidget(QWidget* parent)
	: QWidget(parent)
{
	initLayout();
}

WeatherHistoryWidget::~WeatherHistoryWidget()
{
	// hi
}

void WeatherHistoryWidget::onWeatherData(const WeatherData& data)
{
	if (_wind_rain_chart && _sun_chart)
	{
		_wind_rain_chart->onWeatherData(data);
		_sun_chart->onWeatherData(data);
	}
}

void WeatherHistoryWidget::initLayout()
{
	auto layout = new QVBoxLayout(this);
	setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	// Create tab widget
	_tab_widget = new QTabWidget(this);
	layout->addWidget(_tab_widget);

	// Init wind/rain chart
	_wind_rain_chart = new WindRainChartWidget(history_length_sec, this);
	_tab_widget->addTab(_wind_rain_chart, "Wind & Rain");

	// Init sun chart
	_sun_chart = new SunChartWidget(history_length_sec, this);
	_tab_widget->addTab(_sun_chart, "Sunlight");
}

void WeatherHistoryWidget::updateDisplay(const WeatherData& data)
{
}
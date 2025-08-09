#include "WeatherStationWidget.h"
#include "WindRainChartWidget.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTabWidget>

WeatherStationWidget::WeatherStationWidget(QWidget* parent)
	: QWidget(parent)
{
	initLayout();
}

WeatherStationWidget::~WeatherStationWidget()
{
}

void WeatherStationWidget::onWeatherData(const WeatherData& data)
{
	if (_wind_rain_chart)
		_wind_rain_chart->onWeatherData(data);
}

void WeatherStationWidget::initLayout()
{
	auto layout = new QVBoxLayout(this);
	setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	// Create tab widget
	_tab_widget = new QTabWidget(this);
	layout->addWidget(_tab_widget);

	// Init wind/rain chart
	_wind_rain_chart = new WindRainChartWidget(900, this);
	_tab_widget->addTab(_wind_rain_chart, "Wind & Rain");
}

void WeatherStationWidget::updateDisplay(const WeatherData& data)
{
}
#include "WeatherStationWidget.h"
#include "WindWheelWidget.h"
#include "SunPlotWidget.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>

WeatherStationWidget::WeatherStationWidget(QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_TranslucentBackground);
	setStyleSheet("background: transparent;");

	initLayout();
}

WeatherStationWidget::~WeatherStationWidget()
{
}

void WeatherStationWidget::onWeatherData(const WeatherData& data)
{
	updateDisplay(data);
}

void WeatherStationWidget::initLayout()
{
	auto layout = new QHBoxLayout(this);
	setLayout(layout);

	_sun_plot_widget = new SunPlotWidget(this);
	layout->addWidget(_sun_plot_widget);

	_wind_wheel_widget = new WindWheelWidget(this);
	layout->addWidget(_wind_wheel_widget);
}

void WeatherStationWidget::updateDisplay(const WeatherData& data)
{
	_wind_wheel_widget->windSpeedChanged(data.wind);
}
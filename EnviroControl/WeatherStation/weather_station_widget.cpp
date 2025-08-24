#include "WeatherStationWidget.h"
#include "WindWheelWidget.h"
#include "SunPlotWidget.h"
#include "ThermometerWidget.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>

static const int MAX_SUN_INTENSITY = 99; // kLux

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

	_thermometer_widget = new ThermometerWidget(this);
	layout->addWidget(_thermometer_widget);
}

void WeatherStationWidget::updateDisplay(const WeatherData& data)
{
	_wind_wheel_widget->windSpeedChanged(data.wind);

	// Sun plot widget only gets relative values (0...1)
	_sun_plot_widget->onSunDataChanged(data.sun_south / MAX_SUN_INTENSITY, data.sun_east / MAX_SUN_INTENSITY, data.sun_west / MAX_SUN_INTENSITY);

	_thermometer_widget->temperatureChanged(data.temperature);
}
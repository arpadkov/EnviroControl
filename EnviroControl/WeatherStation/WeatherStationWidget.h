#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>

#include "WeatherData.h"

class WindWheelWidget;
class SunPlotWidget;
class ThermometerWidget;

class WeatherStationWidget : public QWidget
{
	Q_OBJECT

public:
	explicit WeatherStationWidget(QWidget* parent = nullptr);
	~WeatherStationWidget();

public Q_SLOTS:
	void onWeatherData(const WeatherData& data);

private:
	void initLayout();
	void updateDisplay(const WeatherData& data);

	QPointer<WindWheelWidget> _wind_wheel_widget;
	QPointer<SunPlotWidget> _sun_plot_widget;
	QPointer<ThermometerWidget> _thermometer_widget;
};
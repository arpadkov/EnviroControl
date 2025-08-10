#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>

#include "WeatherData.h"

class QTabWidget;

class WindRainChartWidget;

class WeatherHistoryWidget : public QWidget
{
	Q_OBJECT

public:
	explicit WeatherHistoryWidget(QWidget* parent = nullptr);
	~WeatherHistoryWidget();

public Q_SLOTS:
	void onWeatherData(const WeatherData& data);

private:
	void initLayout();
	void updateDisplay(const WeatherData& data);

private:
	QPointer<QTabWidget> _tab_widget;
	QPointer<WindRainChartWidget> _wind_rain_chart;
};
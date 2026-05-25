#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>

#include "WeatherData.h"

class QTabWidget;

class WindRainChartWidget;
class SunChartWidget;

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

private:
	std::shared_ptr<std::vector<WeatherData>> _weather_history;
	int _history_length_sec;

	QPointer<QTabWidget> _tab_widget;
	QPointer<WindRainChartWidget> _wind_rain_chart;
	QPointer<SunChartWidget> _sun_chart;
};
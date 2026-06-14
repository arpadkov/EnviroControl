#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>
#include <vector>

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

		// Buffer holding fine-grained incoming WeatherData for a short period before aggregation
		std::vector<WeatherData> _short_buffer;

	QPointer<QTabWidget> _tab_widget;
	QPointer<WindRainChartWidget> _wind_rain_chart;
	QPointer<SunChartWidget> _sun_chart;
};
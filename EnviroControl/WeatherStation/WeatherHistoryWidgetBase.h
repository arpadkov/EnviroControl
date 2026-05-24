#pragma once

#include "WeatherData.h"

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>

class QChart;
class QChartView;
class QLineSeries;
class QAreaSeries;

class WeatherHistoryWidgetBase : public QWidget
{
public:
	explicit WeatherHistoryWidgetBase(QWidget* parent = nullptr);
	~WeatherHistoryWidgetBase();

public Q_SLOTS:
	void onWeatherData(const WeatherData& data);

protected:
	virtual void setupChart() = 0;
	virtual void updateCharts() = 0;

	// WeatherHistoryChart tools
	static void adjustXAxisRange(QChart* chart, const std::vector<WeatherData>& history);

	QPointer<QChart> _chart;
	QPointer<QChartView> _chart_view;

	std::shared_ptr<std::vector<WeatherData>> _weather_history;
	int _history_length_sec;
	int _display_length_sec;
};

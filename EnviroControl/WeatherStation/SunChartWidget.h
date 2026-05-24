#pragma once

#include "WeatherHistoryWidgetBase.h"

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtCore/QPointF>
#include <QtWidgets/QGraphicsSimpleTextItem>

class QChart;
class QChartView;
class QLineSeries;
class QPointF;
//class QVector<QPointF>;

// Simple single-line chart widget used by SunChartWidget.
// Declared here, implemented in sun_chart_widget.cpp
class SingleSunChart : public WeatherHistoryWidgetBase
{
public:
	explicit SingleSunChart(const QString& title = QString(), QWidget* parent = nullptr);
	~SingleSunChart();

	void setTitle(const QString& title);
	void setPoints(const QVector<QPointF>& points); // x = ms since epoch
	void setGetPointFunc(std::function<double(const WeatherData&)> func);

	QChart* chart() const;

private:
	void setupChart() override;
	void updateCharts() override;

private:
	QLineSeries* _upper_series;
	QLineSeries* _lower_series;
	QAreaSeries* _area_series;
	QGraphicsSimpleTextItem* _title_item;

	// function to determine, which points are displayed (south/east/west)
	std::function<double(const WeatherData&)> _get_point_func;
};

class SunChartWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SunChartWidget(int history_length_sec, QWidget* parent = nullptr);
	~SunChartWidget();

	void onWeatherData(const WeatherData& data);

private:

	QPointer<SingleSunChart> _south_chart;
	QPointer<SingleSunChart> _east_chart;
	QPointer<SingleSunChart> _west_chart;
};

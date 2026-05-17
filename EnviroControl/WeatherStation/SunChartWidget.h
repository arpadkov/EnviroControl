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

// Simple single-line chart widget used by SunChartWidget.
// Declared here, implemented in sun_chart_widget.cpp
class SingleSunChart : public QWidget
{
public:
		explicit SingleSunChart(const QString& title = QString(), QWidget* parent = nullptr);
		~SingleSunChart();

		void setTitle(const QString& title);
		void setPoints(const QVector<QPointF>& points); // x = ms since epoch

		QChart* chart() const;

private:
		QPointer<QChart> _chart;
		QPointer<QChartView> _chart_view;
		QLineSeries* _upper_series;
		QLineSeries* _lower_series;
		QAreaSeries* _area_series;
		QGraphicsSimpleTextItem* _title_item;
};

class SunChartWidget : public WeatherHistoryWidgetBase
{
		Q_OBJECT

public:
		explicit SunChartWidget(int history_length_sec, QWidget* parent = nullptr);
		~SunChartWidget();

private:
		void setupChart() override;
		void updateCharts() override;

		QPointer<SingleSunChart> _south_chart;
		QPointer<SingleSunChart> _east_chart;
		QPointer<SingleSunChart> _west_chart;
};

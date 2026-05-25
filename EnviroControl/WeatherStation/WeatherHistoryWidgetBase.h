#pragma once

#include "WeatherData.h"

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>
#include <QtCharts/QChartView>
#include <QtGui/QColor>

class QChart;
class QLineSeries;
class QAreaSeries;

class ScrollableChartView : public QChartView
{
public:
	explicit ScrollableChartView(QChart* chart, QWidget* parent = nullptr);
	~ScrollableChartView();

	void setDataRange(const QDateTime& min, const QDateTime& max);

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	bool _dragging;
	QPoint _last_pos;

	QDateTime _data_min;
	QDateTime _data_max;
};

class WeatherHistoryWidgetBase : public QWidget
{
public:
	explicit WeatherHistoryWidgetBase(std::shared_ptr<std::vector<WeatherData>> weather_history, QWidget* parent = nullptr);
	~WeatherHistoryWidgetBase();

	// Helper to set a gradient fill on an area series. The topColor will be used
	// as the opaque color at the top; a transparent variant will be used at the bottom.
	static void setAreaSeriesFill(QAreaSeries* area, const QColor& topColor);

public Q_SLOTS:
	void onWeatherData();

protected:
	virtual void setupChart() = 0;
	virtual void updateCharts() = 0;

	void showEvent(QShowEvent* event) override;

	void adjustXAxisRange(bool force = false);

	QPointer<QChart> _chart;
	QPointer<ScrollableChartView> _chart_view;

	std::shared_ptr<std::vector<WeatherData>> _weather_history;
	int _display_length_sec;
};

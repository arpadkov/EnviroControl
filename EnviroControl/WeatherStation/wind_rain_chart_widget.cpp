#include "WindRainChartWidget.h"

#include <QtWidgets/QVBoxLayout>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QAreaSeries>

WindRainChartWidget::WindRainChartWidget(int history_length_sec, QWidget* parent)
	: WeatherHistoryWidgetBase(parent),
	_wind_series(new QLineSeries())
{
	setupChart();
	auto layout = new QVBoxLayout(this);
	layout->addWidget(_chart_view);
	setLayout(layout);

	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
}

WindRainChartWidget::~WindRainChartWidget()
{
}

void WindRainChartWidget::setupChart()
{
	//_chart->setTitle("Wind and Rain");
	_chart->legend()->show();

	// Wind series
	_wind_series->setName("Wind (m/s)");
	_chart->addSeries(_wind_series);
	QPen wind_pen;
	wind_pen.setColor(Qt::blue); // Set a visible color
	wind_pen.setWidth(2);        // Set a visible width
	_wind_series->setPen(wind_pen);

	// Rain series
	_rain_lower_series = new QLineSeries();
	_rain_upper_series = new QLineSeries();
	_rain_series = new QAreaSeries(_rain_upper_series, _rain_lower_series);
	_rain_series->setName("Rain");

	// Set a semi-transparent blue color for the filled area
	QColor area_color("#87CEEB");
	area_color.setAlphaF(0.3);
	_rain_series->setColor(area_color);

	// Make the boundary lines invisible
	QPen transparentPen(Qt::transparent);
	_rain_upper_series->setPen(transparentPen);
	_rain_lower_series->setPen(transparentPen);

	_chart->addSeries(_rain_series);

	// X-Axis
	auto x_axis = new QDateTimeAxis();
	x_axis->setFormat("hh:mm:ss");
	_chart->addAxis(x_axis, Qt::AlignBottom);
	_wind_series->attachAxis(x_axis);
	_rain_series->attachAxis(x_axis);

	// Y-Axis for Wind
	auto wind_axis = new QValueAxis();
	wind_axis->setRange(0, 20);
	_chart->addAxis(wind_axis, Qt::AlignLeft);
	_wind_series->attachAxis(wind_axis);

	// Y-Axis for Rain (hidden)
	auto rain_axis = new QValueAxis();
	rain_axis->setVisible(false);
	_chart->addAxis(rain_axis, Qt::AlignRight);
	_rain_series->attachAxis(rain_axis);
}

void WindRainChartWidget::updateCharts()
{
	_wind_series->clear();
	_rain_upper_series->clear();
	_rain_lower_series->clear();

	for (const auto& data : *_weather_history)
	{
		// Add wind data
		_wind_series->append(data.timestamp.toMSecsSinceEpoch(), data.wind);
		// Add rain data
		_rain_lower_series->append(data.timestamp.toMSecsSinceEpoch(), 0);
		_rain_upper_series->append(data.timestamp.toMSecsSinceEpoch(), data.rain ? 1 : 0);
	}

	adjustXAxisRange(_chart, *_weather_history);
}

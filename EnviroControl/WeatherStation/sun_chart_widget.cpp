#include "SunChartWidget.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QAreaSeries>

// SingleSunChart
SingleSunChart::SingleSunChart(const QString& title, QWidget* parent)
	: QWidget(parent), _chart(new QChart()), _chart_view(new QChartView(_chart, this)), _series(new QLineSeries())
{
	_chart->setTitle(title);
	_chart->legend()->hide();
	QPen pen(Qt::yellow);
	pen.setWidth(2);
	_series->setPen(pen);
	_chart->addSeries(_series);

	auto x = new QDateTimeAxis();
	x->setFormat("hh:mm:ss");
	_chart->addAxis(x, Qt::AlignBottom);
	_series->attachAxis(x);

	auto y = new QValueAxis();
	y->setRange(0, 100);
	_chart->addAxis(y, Qt::AlignLeft);
	_series->attachAxis(y);

	auto l = new QVBoxLayout(this);
	l->setContentsMargins(0, 0, 0, 0);
	l->addWidget(_chart_view);
	setLayout(l);
}

QChart* SingleSunChart::chart() const
{
	return _chart;
}

SingleSunChart::~SingleSunChart()
{}

void SingleSunChart::setTitle(const QString& title)
{
	if (_chart)
		_chart->setTitle(title);
}

void SingleSunChart::setPoints(const QVector<QPointF>& points)
{
	if (!_series)
		return;
	_series->clear();
	for (const auto& p : points)
		_series->append(p);
}

// SunChartWidget
SunChartWidget::SunChartWidget(int history_length_sec, QWidget* parent)
	: WeatherHistoryWidgetBase(history_length_sec, parent),
	_south_chart(new SingleSunChart("South", this)),
	_east_chart(new SingleSunChart("East", this)),
	_west_chart(new SingleSunChart("West", this))
{
	setupChart();

	auto layout = new QVBoxLayout(this);
	layout->addWidget(_south_chart);
	layout->addWidget(_east_chart);
	layout->addWidget(_west_chart);
	setLayout(layout);

	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
}

SunChartWidget::~SunChartWidget()
{}

void SunChartWidget::setupChart()
{
	// nothing here; charts are created individually
}

void SunChartWidget::updateCharts()
{
	QVector<QPointF> south_points;
	QVector<QPointF> east_points;
	QVector<QPointF> west_points;

	qint64 min_ts = std::numeric_limits<qint64>::max();
	qint64 max_ts = 0;

	for (const auto& data : _weather_history)
	{
		const qint64 t = data.timestamp.toMSecsSinceEpoch();
		south_points.append(QPointF(t, data.sun_south));
		east_points.append(QPointF(t, data.sun_east));
		west_points.append(QPointF(t, data.sun_west));
		min_ts = std::min(min_ts, t);
		max_ts = std::max(max_ts, t);
	}

	if (_south_chart)
		_south_chart->setPoints(south_points);
	if (_east_chart)
		_east_chart->setPoints(east_points);
	if (_west_chart)
		_west_chart->setPoints(west_points);

	// Use shared utility to adjust X axis range on each chart
	if (min_ts <= max_ts)
	{
		if (_south_chart && _south_chart->chart())
			WeatherHistoryWidgetBase::adjustXAxisRange(_south_chart->chart(), _weather_history);
		if (_east_chart && _east_chart->chart())
			WeatherHistoryWidgetBase::adjustXAxisRange(_east_chart->chart(), _weather_history);
		if (_west_chart && _west_chart->chart())
			WeatherHistoryWidgetBase::adjustXAxisRange(_west_chart->chart(), _weather_history);
	}
}


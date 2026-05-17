#include "SunChartWidget.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QSizePolicy>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QAreaSeries>
#include <QtGui/QLinearGradient>
#include <QtCore/QMargins>

// SingleSunChart
SingleSunChart::SingleSunChart(const QString& title, QWidget* parent)
	: QWidget(parent), _chart(new QChart()), _chart_view(new QChartView(_chart, this)), _upper_series(new QLineSeries(_chart)), _lower_series(new QLineSeries(_chart)), _area_series(nullptr)
{
	_chart->legend()->hide();

	// reduce default margins so plot area uses more vertical space
	_chart->setMargins(QMargins(0, 0, 0, 0));
	// also remove any internal contents margins and title space the QChart might reserve
	_chart->setContentsMargins(QMargins(0, 0, 0, 0));
	_chart_view->setContentsMargins(0, 0, 0, 0);

	// Prepare area series: lower is baseline at 0 (will be set in setPoints)
	_area_series = new QAreaSeries(_upper_series, _lower_series);
	_area_series->setName(title);

	// Gradient fill: yellow at top to transparent at bottom
	QLinearGradient gradient;
	gradient.setStart(0, 0);
	gradient.setFinalStop(0, 1);
	gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
	QColor topColor(Qt::yellow);
	topColor.setAlphaF(0.9);
	QColor bottomColor(Qt::yellow);
	bottomColor.setAlphaF(0.0);
	gradient.setColorAt(0.0, topColor);
	gradient.setColorAt(1.0, bottomColor);
	_area_series->setBrush(gradient);
	_area_series->setPen(QPen(Qt::NoPen));
	_chart->addSeries(_area_series);
	_chart_view->setRenderHint(QPainter::Antialiasing);

	auto x = new QDateTimeAxis();
	x->setFormat("hh:mm:ss");
	_chart->addAxis(x, Qt::AlignBottom);
	_area_series->attachAxis(x);

	auto y = new QValueAxis();
	y->setRange(0, 100);
	_chart->addAxis(y, Qt::AlignLeft);
	_area_series->attachAxis(y);

	// create title item inside chart (top-left)
	_title_item = new QGraphicsSimpleTextItem(title);
	if (_chart->scene())
		_chart->scene()->addItem(_title_item);
	// small margins -> remove to use full widget area
	auto l = new QVBoxLayout(this);
	l->setContentsMargins(0, 0, 0, 0);
	l->setSpacing(0);

	// remove frame and scrollbars from the chart view so stacked charts sit flush
	_chart_view->setFrameStyle(QFrame::NoFrame);
	_chart_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_chart_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_chart_view->setContentsMargins(0, 0, 0, 0);
	_chart_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	l->addWidget(_chart_view, /*stretch*/ 1);
	setLayout(l);

	// ensure the SingleSunChart widget itself has no extra margins and expands
	setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
	if (!_area_series || !_upper_series || !_lower_series)
		return;

	_upper_series->clear();
	_lower_series->clear();

	for (const auto& p : points)
	{
		// p.x() is ms since epoch
		_upper_series->append(p);
		_lower_series->append(QPointF(p.x(), 0));
	}

	// If chart axes exist, ensure area series is attached (already added in ctor)
	// Trigger chart update
	_area_series->setUpperSeries(_upper_series);
	_area_series->setLowerSeries(_lower_series);

	// position title at top-left inside chart plot area
	if (_title_item && _chart->plotArea().isValid())
	{
		const QRectF area = _chart->plotArea();
		const QPointF pos(area.left() + 4, area.top());
		_title_item->setPos(pos);
	}
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


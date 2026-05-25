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
#include <QtCore/QMargins>

// SingleSunChart
SingleSunChart::SingleSunChart(std::shared_ptr<std::vector<WeatherData>> weather_history, const QString& title, QWidget* parent)
	: WeatherHistoryWidgetBase(weather_history, parent), _upper_series(new QLineSeries(_chart)), _lower_series(new QLineSeries(_chart)), _area_series(nullptr)
{
	_chart->legend()->hide();

	// Prepare area series: lower is baseline at 0 (will be set in setPoints)
	_area_series = new QAreaSeries(_upper_series, _lower_series);
	_area_series->setName(title);

		// Set gradient fill using helper
		WeatherHistoryWidgetBase::setAreaSeriesFill(_area_series, QColor(Qt::yellow));
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

void SingleSunChart::setupChart()
{}

void SingleSunChart::updateCharts()
{
	QElapsedTimer timer;
	timer.start();

	QVector<QPointF> points;
	for (const auto& data : *_weather_history)
	{
		const qint64 t = data.timestamp.toMSecsSinceEpoch();
		points.append(QPointF(t, _get_point_func(data)));
	}

	setPoints(points);

	qDebug() << " SINGLE SUN CHART UPDATE TOOK" << timer.elapsed() << "ms for" << points.size() << "points";
	WeatherHistoryWidgetBase::adjustXAxisRange();
	qDebug() << " SINGLE SUN CHART ADJUST X-AXIS TOOK" << timer.elapsed() << "ms";
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

void SingleSunChart::setGetPointFunc(std::function<double(const WeatherData&)> func)
{
	_get_point_func = func;
}

// SunChartWidget
SunChartWidget::SunChartWidget(std::shared_ptr<std::vector<WeatherData>> weather_history, QWidget* parent)
	: QWidget(parent),
	_south_chart(new SingleSunChart(weather_history, "South", this)),
	_east_chart(new SingleSunChart(weather_history, "East", this)),
	_west_chart(new SingleSunChart(weather_history, "West", this))
{
	_south_chart->setGetPointFunc([](const WeatherData& data)	{return data.sun_south;});
	_east_chart->setGetPointFunc([](const WeatherData& data) {return data.sun_east;});
	_west_chart->setGetPointFunc([](const WeatherData& data) {return data.sun_west;});

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

void SunChartWidget::onWeatherData()
{
	_south_chart->onWeatherData();
	_east_chart->onWeatherData();
	_west_chart->onWeatherData();
}


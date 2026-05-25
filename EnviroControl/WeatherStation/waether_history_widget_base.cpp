#include "WeatherHistoryWidgetBase.h"

#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QChart>
#include <QtCharts/QXYSeries>

static const int DEFAULT_DISPLAY_LENGTH_SEC = 900;

ScrollableChartView::ScrollableChartView(QChart* chart, QWidget* parent)
	: QChartView(chart, parent), _dragging(false)
{
	setRubberBand(QChartView::NoRubberBand);
	setMouseTracking(true);
}

ScrollableChartView::~ScrollableChartView()
{}

void ScrollableChartView::setDataRange(const QDateTime& min, const QDateTime& max)
{
	_data_min = min;
	_data_max = max;
}

void ScrollableChartView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		_dragging = true;
		_last_pos = event->pos();
		setCursor(Qt::DragMoveCursor);
	}

	QChartView::mousePressEvent(event);
}

void ScrollableChartView::mouseMoveEvent(QMouseEvent* event)
{
	if (_dragging)
	{
		QPoint delta = event->pos() - _last_pos;
		//chart()->scroll(-delta.x(), 0); // Scroll horizontally based on mouse movement

		auto x_axis = qobject_cast<QDateTimeAxis*>(chart()->axes(Qt::Horizontal).at(0));
		if (x_axis)
		{
			qint64 visible_ms = x_axis->min().msecsTo(x_axis->max());

			QRectF plot = chart()->plotArea();
			double ms_per_pixel = visible_ms / plot.width();
			qint64 delta_ms = static_cast<qint64>(delta.x() * ms_per_pixel);

			QDateTime new_min = x_axis->min().addMSecs(-delta_ms);
			QDateTime new_max = x_axis->max().addMSecs(-delta_ms);

			// If visible is larger than data range, lock to data range
			if (visible_ms >= _data_min.msecsTo(_data_max))
			{
				new_min = _data_min;
				new_max = _data_max;
			}
			else
			{
				// Clamp left
				if (new_min < _data_min)
				{
					new_min = _data_min;
					new_max = new_min.addMSecs(visible_ms);
				}

				// Clamp right
				if (new_max > _data_max)
				{
					new_max = _data_max;
					new_min = new_max.addMSecs(-visible_ms);
				}
			}

			x_axis->setRange(new_min, new_max);
		}

		_last_pos = event->pos();
	}

	QChartView::mouseMoveEvent(event);
}

void ScrollableChartView::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		_dragging = false;
		setCursor(Qt::ArrowCursor);
	}

	QChartView::mouseReleaseEvent(event);
}

WeatherHistoryWidgetBase::WeatherHistoryWidgetBase(std::shared_ptr<std::vector<WeatherData>> weather_history, QWidget* parent)
	: QWidget(parent), _weather_history(weather_history), _display_length_sec(DEFAULT_DISPLAY_LENGTH_SEC),
	_chart(new QChart()), _chart_view(new ScrollableChartView(_chart, this))
{
	_chart->setMargins(QMargins(0, 0, 0, 0)); // Set all margins to 0
	_chart->setContentsMargins(0, 0, 0, 0);  // Set content margins to 0

	_chart->setAnimationOptions(QChart::NoAnimation);
}

WeatherHistoryWidgetBase::~WeatherHistoryWidgetBase()
{}

void WeatherHistoryWidgetBase::onWeatherData()
{
	_chart_view->setDataRange(_weather_history->front().timestamp, _weather_history->back().timestamp);
	updateCharts();
}

void WeatherHistoryWidgetBase::showEvent(QShowEvent* event)
{
	adjustXAxisRange(true);
	QWidget::showEvent(event);
}

void WeatherHistoryWidgetBase::adjustXAxisRange(bool force)
{
	auto x_axis = qobject_cast<QDateTimeAxis*>(_chart->axes(Qt::Horizontal).at(0));
	if (!x_axis)
		return;

	QDateTime first = _weather_history->front().timestamp;
	QDateTime last = _weather_history->back().timestamp;
	qint64 available_ms = first.msecsTo(last);

	auto set_default_range = [&]()
		{
			qint64 now = QDateTime::currentMSecsSinceEpoch();
			x_axis->setRange(QDateTime::fromMSecsSinceEpoch(now - _display_length_sec * 1000), QDateTime::fromMSecsSinceEpoch(now));
		};

	// If there is not enough data to fill the graph -> lock to data range
	if (force || available_ms < _display_length_sec * 1000)
	{
		set_default_range();
		return;
	}

	// If the user scrolled back in time, skip range update
	if (x_axis->max() < last.addSecs(-10))
		return;

	// Otherwise update to show the latest data
	set_default_range();
}

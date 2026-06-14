#include "WeatherHistoryWidget.h"
#include "WindRainChartWidget.h"
#include "SunChartWidget.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTabWidget>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtWidgets/QTabBar>
#include <QtGui/QIcon>
#include <QtCore/QSize>
#include <QtGui/QPixmap>
#include <QtGui/QTransform>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>

static const int DEFAULT_HISTORY_LENGTH_SEC = 1800;

namespace
{
static const int SHORT_BUFFER_SEC = 10;

// Compute averaged WeatherData from a buffer of samples.
static WeatherData computeAveragedWeatherData(const std::vector<WeatherData>& buf)
{
	WeatherData a{};
	double t_sum = 0.0;
	double ss_sum = 0.0;
	double se_sum = 0.0;
	double sw_sum = 0.0;
	double daylight_sum = 0.0;
	double wind_sum = 0.0;
	bool twighlight_any = false;
	bool rain_any = false;

	for (const auto& s : buf)
	{
		t_sum += s.temperature;
		ss_sum += s.sun_south;
		se_sum += s.sun_east;
		sw_sum += s.sun_west;
		daylight_sum += s.daylight;
		wind_sum += s.wind;
		twighlight_any = twighlight_any || s.twighlight;
		rain_any = rain_any || s.rain;
	}

	double count = static_cast<double>(buf.size());
	a.temperature = t_sum / count;
	a.sun_south = ss_sum / count;
	a.sun_east = se_sum / count;
	a.sun_west = sw_sum / count;
	a.daylight = daylight_sum / count;
	a.wind = wind_sum / count;
	a.twighlight = twighlight_any;
	a.rain = rain_any;
	a.timestamp = buf.back().timestamp;
	return a;
}

// Process the short-term buffer: remove too-old entries, and when the buffer spans at
// least SHORT_BUFFER_SEC seconds, compute an averaged WeatherData entry, push it to
// the main history and trim the main history to the configured duration.
bool processShortBuffer(std::vector<WeatherData>& short_buffer,
	const WeatherData& latest,
	std::shared_ptr<std::vector<WeatherData>> history,
	int history_length_sec)
{
	if (short_buffer.empty())
		return false;

	QDateTime earliest = short_buffer.front().timestamp;
	qint64 span = earliest.msecsTo(short_buffer.back().timestamp);
	if (span < (qint64)SHORT_BUFFER_SEC * 1000)
		return false;

	// Compute averaged sample and push to history
	WeatherData avg = computeAveragedWeatherData(short_buffer);
	history->push_back(avg);

	// Clear short buffer after aggregation
	short_buffer.clear();

	// Limit history to the specified time duration length
	QDateTime oldest_to_keep = latest.timestamp.addSecs(-history_length_sec);
	while (!history->empty() && history->front().timestamp < oldest_to_keep)
		history->erase(history->begin());

	return true;
}

QString getStyleSheet(const QString& file_name)
{
	QFile style_file(QString(":/styles/style_sheets/%1.qss").arg(file_name));
	if (style_file.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream stream(&style_file);
		QString style_sheet = stream.readAll();
		style_file.close();
		return style_sheet;
	}
	return QString();
}

void stylaTab(int id, const QString& icon_path, QPointer<QTabWidget> tab_widget)
{
	QPixmap pix(icon_path);
	tab_widget->setTabIcon(id, QIcon(pix));
	tab_widget->setTabText(id, QString());
	tab_widget->setIconSize(tab_widget->tabBar()->sizeHint());
	tab_widget->tabBar()->setDrawBase(false);
}

}

WeatherHistoryWidget::WeatherHistoryWidget(QWidget* parent)
	: QWidget(parent), _history_length_sec(DEFAULT_HISTORY_LENGTH_SEC)
{
	_weather_history = std::make_shared<std::vector<WeatherData>>();

	initLayout();
}

WeatherHistoryWidget::~WeatherHistoryWidget()
{}

void WeatherHistoryWidget::onWeatherData(const WeatherData& data)
{
	// Append incoming sample to short-term buffer and process/aggregate when needed
	_short_buffer.push_back(data);
	bool pushed = processShortBuffer(_short_buffer, data, _weather_history, _history_length_sec);

	// If a new averaged sample was pushed to history, update charts
	if (pushed && !_weather_history->empty())
	{
		_wind_rain_chart->onWeatherData();
		_sun_chart->onWeatherData();
	}
}

void WeatherHistoryWidget::initLayout()
{
	auto layout = new QVBoxLayout(this);
	setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	// Create tab widget
	_tab_widget = new QTabWidget(this);
	// Place the tab bar on the left for a vertical icon-only navigation
	_tab_widget->setTabPosition(QTabWidget::West);
	_tab_widget->setDocumentMode(true); // give a modern flat look on some platforms
	_tab_widget->setStyleSheet(getStyleSheet("history_tabs"));
	if (auto tb = _tab_widget->tabBar())
	{
		tb->setIconSize(QSize(36, 36));
		tb->setExpanding(false);
		tb->setMovable(false);
		tb->setTabsClosable(false);
		tb->setContentsMargins(0, 0, 0, 0);
	}
	layout->addWidget(_tab_widget);

	// Init wind/rain chart
	_wind_rain_chart = new WindRainChartWidget(_weather_history, this);
	int id_wind_rain = _tab_widget->addTab(_wind_rain_chart, "");
	stylaTab(id_wind_rain, ":/main_tabs/icons/close.svg", _tab_widget);

	// Init sun chart
	_sun_chart = new SunChartWidget(_weather_history, this);
	int id_sun = _tab_widget->addTab(_sun_chart, "");
	stylaTab(id_sun, ":/main_tabs/icons/close.svg", _tab_widget);
}
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

static const int history_length_sec = 900; // 15 minutes

namespace
{
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
	: QWidget(parent)
{
	initLayout();
}

WeatherHistoryWidget::~WeatherHistoryWidget()
{}

void WeatherHistoryWidget::onWeatherData(const WeatherData& data)
{
	if (_wind_rain_chart && _sun_chart)
	{
		_wind_rain_chart->onWeatherData(data);
		_sun_chart->onWeatherData(data);
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
	_wind_rain_chart = new WindRainChartWidget(history_length_sec, this);
	int id_wind_rain = _tab_widget->addTab(_wind_rain_chart, "");
	stylaTab(id_wind_rain, ":/main_tabs/icons/close.svg", _tab_widget);

	// Init sun chart
	_sun_chart = new SunChartWidget(history_length_sec, this);
	int id_sun = _tab_widget->addTab(_sun_chart, "");
	stylaTab(id_sun, ":/main_tabs/icons/close.svg", _tab_widget);
}
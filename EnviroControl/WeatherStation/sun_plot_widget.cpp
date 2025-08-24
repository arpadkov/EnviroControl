#include "SunPlotWidget.h"

#include <QtQml/QQmlContext>
#include <QtWidgets/QVBoxLayout>
#include <QtQuickWidgets/QQuickWidget>

SunPlotWidget::SunPlotWidget(QWidget* parent)
	: QWidget(parent), _qml_widget(new QQuickWidget(this)), _sun_plot_data(new SunPlotData(this))
{
	_qml_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	_qml_widget->rootContext()->setContextProperty("sunPlotData", _sun_plot_data);
	_qml_widget->setSource(QUrl("qrc:/WeatherStation/qml_resources/SunPlot.qml"));
	initLayout();
}

SunPlotWidget::~SunPlotWidget()
{
}

void SunPlotWidget::onSunDataChanged(double south, double east, double west)
{
	if (_sun_plot_data)
	{
		_sun_plot_data->setSunSouth(south);
		_sun_plot_data->setSunEast(east);
		_sun_plot_data->setSunWest(west);
	}
}

void SunPlotWidget::initLayout()
{
	auto layout = new QVBoxLayout(this);
	setLayout(layout);
	_qml_widget->setClearColor(Qt::transparent); // Set background to transparent
	_qml_widget->setMinimumSize(300, 300); // Set minimum size for the widget
	layout->addWidget(_qml_widget);
}


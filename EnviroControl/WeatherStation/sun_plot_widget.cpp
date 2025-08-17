#include "SunPlotWidget.h"

#include <QtQml/QQmlContext>
#include <QtWidgets/QVBoxLayout>
#include <QtQuickWidgets/QQuickWidget>

SunPlotWidget::SunPlotWidget(QWidget* parent)
	: QWidget(parent), _qml_widget(new QQuickWidget(this))
{
	_qml_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	_qml_widget->setSource(QUrl("qrc:/WeatherStation/qml_resources/SunPlot.qml"));
	//_qml_widget->rootContext()->setContextProperty("sunPlotWidget", this);
	initLayout();
	//setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
	//setFixedSize(300, 300);
}

SunPlotWidget::~SunPlotWidget()
{
}

void SunPlotWidget::onSunDataChanged(double south, double east, double west)
{
	if (_qml_widget)
	{
		//_qml_widget->rootObject()->setProperty("south", south);
		//_qml_widget->rootObject()->setProperty("east", east);
		//_qml_widget->rootObject()->setProperty("west", west);
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


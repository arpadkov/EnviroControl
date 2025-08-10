#include "WindWheelWidget.h"

#include <QtQml/QQmlContext>
#include <QtWidgets/QVBoxLayout>

WindWheelWidget::WindWheelWidget(QWidget* parent)
	: QWidget(parent),
	_qml_widget(new QQuickWidget(this)), _wind_wheel_data(new WindWheelData(this))
{
	_qml_widget->rootContext()->setContextProperty("windWheelData", _wind_wheel_data);
	_qml_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	_qml_widget->setSource(QUrl("qrc:/WeatherStation/qml_resources/WindWheel.qml"));
	//_qml_widget->setSource(QUrl());

	initLayout();
}

WindWheelWidget::~WindWheelWidget()
{
}

void WindWheelWidget::windSpeedChanged(double speed)
{
	if (_wind_wheel_data)
		_wind_wheel_data->setWindSpeed(speed);
}

void WindWheelWidget::initLayout()
{
	auto layout = new QVBoxLayout(this);
	setLayout(layout);
	_qml_widget->setClearColor(Qt::transparent); // Set background to transparent
	layout->addWidget(_qml_widget);
}
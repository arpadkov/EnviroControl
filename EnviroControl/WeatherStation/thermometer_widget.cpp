#include "ThermometerWidget.h"

#include <QtQml/QQmlContext>
#include <QtWidgets/QVBoxLayout>

ThermometerWidget::ThermometerWidget(QWidget* parent)
	: QWidget(parent), 
	_qml_widget(new QQuickWidget(this)), _temperature_data(new ThermometerData(this))
{
  _qml_widget->rootContext()->setContextProperty("thermometerData", _temperature_data);
  _qml_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	_qml_widget->setSource(QUrl("qrc:/WeatherStation/qml_resources/Thermometer.qml"));

	initLayout();
}

ThermometerWidget::~ThermometerWidget()
{
}

void ThermometerWidget::temperatureChanged(double temperature)
{
	_temperature_data->setTemperature(temperature);
}

void ThermometerWidget::initLayout()
{
	auto layout = new QVBoxLayout(this);
	setLayout(layout);
	layout->addWidget(_qml_widget);
}
#include "IndoorStationWidget.h"
#include "IndoorStation.h"
#include "ThermometerWidget.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtCore/QFile>

IndoorStationWidget::IndoorStationWidget(QWidget* parent)
	: QFrame(parent)
{
	initLayout();
}

IndoorStationWidget::~IndoorStationWidget() = default;

void IndoorStationWidget::onIndoorDataChanged(const IndoorData& data)
{
	//_data_label->setText(data.toString());
	_thermometer_widget->temperatureChanged(data.temperature);
}

void IndoorStationWidget::initLayout()
{
	auto layout = new QVBoxLayout(this);
	setLayout(layout);

	//_data_label = new QLabel(this);
	//layout->addWidget(_data_label);

	_thermometer_widget = new ThermometerWidget(this);
	layout->addWidget(_thermometer_widget);
}
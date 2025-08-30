#include "IndoorStationWidget.h"
#include "IndoorStation.h"

#include <QVBoxLayout>
#include <QLabel>

IndoorStationWidget::IndoorStationWidget(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    _data_label = new QLabel("No data", this);
    layout->addWidget(_data_label);
    setLayout(layout);
}

IndoorStationWidget::~IndoorStationWidget() = default;

void IndoorStationWidget::onIndoorDataChanged(const IndoorData& data)
{
    // Assuming IndoorData has a toString() method
    _data_label->setText(data.toString());
}
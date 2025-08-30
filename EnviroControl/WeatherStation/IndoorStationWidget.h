#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>

class QLabel;

struct IndoorData;

class IndoorStationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IndoorStationWidget(QWidget* parent = nullptr);
    ~IndoorStationWidget() override;

public slots:
    void onIndoorDataChanged(const IndoorData& data);

private:
	QPointer<QLabel> _data_label;
};

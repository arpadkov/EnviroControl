#pragma once

#include <QtWidgets/QFrame>
#include <QtCore/QPointer>

class QLabel;

struct IndoorData;
class ThermometerWidget;

class IndoorStationWidget : public QFrame
{
	Q_OBJECT

public:
	explicit IndoorStationWidget(QWidget* parent = nullptr);
	~IndoorStationWidget() override;

public Q_SLOTS:
	void onIndoorDataChanged(const IndoorData& data);

private:
	void initLayout();

	QPointer<QLabel> _data_label;
	QPointer<ThermometerWidget> _thermometer_widget;
};

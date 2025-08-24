#pragma once

#include <QtWidgets/QWidget>
#include <QtQuickWidgets/QQuickWidget>

class ThermometerData : public QObject
{
	Q_OBJECT
		Q_PROPERTY(double temperature READ temperature WRITE setTemperature NOTIFY temperatureChanged)

public:
	explicit ThermometerData(QObject* parent = nullptr) : QObject(parent), _temperature(0.0)
	{
	};

	double temperature() const
	{
		return _temperature;
	};

	void setTemperature(double temperature)
	{
		_temperature = temperature;
		Q_EMIT temperatureChanged(_temperature);
	};

Q_SIGNALS:
	void temperatureChanged(double temperature);

private:
	double _temperature;
};

class ThermometerWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ThermometerWidget(QWidget* parent = nullptr);
	~ThermometerWidget();

public Q_SLOTS:
	void temperatureChanged(double speed);

private:
	void initLayout();

	QQuickWidget* _qml_widget;
	ThermometerData* _temperature_data;
};
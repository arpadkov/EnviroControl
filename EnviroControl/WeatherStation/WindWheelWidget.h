#pragma once

#include <QtWidgets/QWidget>
#include <QtQuickWidgets/QQuickWidget>

class WindWheelData : public QObject
{
	Q_OBJECT
		Q_PROPERTY(double windSpeed READ windSpeed WRITE setWindSpeed NOTIFY windSpeedChanged)

public:
	explicit WindWheelData(QObject* parent = nullptr) : QObject(parent), _wind_speed(0.0)
	{
	};

	double windSpeed() const
	{
		return _wind_speed;
	};

	void setWindSpeed(double speed)
	{
		_wind_speed = speed;
		Q_EMIT windSpeedChanged(_wind_speed);
	};

Q_SIGNALS:
	void windSpeedChanged(double speed);

private:
	double _wind_speed; // Wind speed in m/s
};

class WindWheelWidget : public QWidget
{
	Q_OBJECT

public:
	explicit WindWheelWidget(QWidget* parent = nullptr);
	~WindWheelWidget();

public Q_SLOTS:
	void windSpeedChanged(double speed);

private:
	void initLayout();

	QQuickWidget* _qml_widget; // For displaying the wind wheel
	WindWheelData* _wind_wheel_data; // Data model for the wind wheel
};
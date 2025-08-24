#pragma once

#include <QtWidgets/QWidget>
#include <QtQml/QtQml>

class QQuickWidget;

class SunPlotData : public QObject
{
	Q_OBJECT
		Q_PROPERTY(double sunSouth READ sunSouth WRITE setSunSouth NOTIFY sunDataChanged)
		Q_PROPERTY(double sunEast READ sunEast WRITE setSunEast NOTIFY sunDataChanged)
		Q_PROPERTY(double sunWest READ sunWest WRITE setSunWest NOTIFY sunDataChanged)

public:
	explicit SunPlotData(QObject* parent = nullptr) :
		QObject(parent), _sun_south(0), _sun_east(0), _sun_west(0)
	{
	};

	double sunSouth() const
	{
		return _sun_south;
	};

	void setSunSouth(double value)
	{
		_sun_south = value;
		Q_EMIT sunDataChanged();
	};

	double sunEast() const
	{
		return _sun_east;
	};

	void setSunEast(double value)
	{
		_sun_east = value;
		Q_EMIT sunDataChanged();
	};

	double sunWest() const
	{
		return _sun_west;
	};

	void setSunWest(double value)
	{
		_sun_west = value;
		Q_EMIT sunDataChanged();
	};

Q_SIGNALS:
	void sunDataChanged();

private:
	double _sun_south; // Proportional to max value (0...1)
	double _sun_east;
	double _sun_west;
};

class SunPlotWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SunPlotWidget(QWidget* parent = nullptr);
	~SunPlotWidget();

public Q_SLOTS:
	void onSunDataChanged(double south, double east, double west);

private:
	void initLayout();

	QQuickWidget* _qml_widget;
	SunPlotData* _sun_plot_data; // Data model
};


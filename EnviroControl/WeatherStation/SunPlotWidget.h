#pragma once

#include <QtWidgets/QWidget>

class QQuickWidget;

class SunPlotWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SunPlotWidget(QWidget* parent = nullptr);
	~SunPlotWidget();

	//bool hasHeightForWidth() const override
	//{
	//	//qDebug() << "HAS HEIGHT FOR WIDTH";
	//	return true;
	//}

	//int heightForWidth(int width) const override
	//{
	//	qDebug() << "HEIGHT FOR WIDTH called with width:" << width;
	//	return width;
	//}

	//QSize sizeHint() const override
	//{
	//	qDebug() << "SIZE HINT called";
	//	return QSize(300, 300); // Default size hint
	//}

public Q_SLOTS:
	void onSunDataChanged(double south, double east, double west);

private:
	void initLayout();

	QQuickWidget* _qml_widget;
};	


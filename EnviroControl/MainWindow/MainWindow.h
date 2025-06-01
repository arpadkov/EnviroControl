#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindowClass;
};
QT_END_NAMESPACE

namespace WFP
{
class ForecastData;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

public Q_SLOTS:
	void onWeatherDataReady(const WFP::ForecastData& data);

private:
	Ui::MainWindowClass* ui;
};


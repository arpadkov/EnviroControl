#include "MainWindow.h"

#include "WeatherStation.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindowClass())
{
	auto ws = new WeatherStation(this);

	ui->setupUi(this);
}

MainWindow::~MainWindow()
{
	delete ui;
}


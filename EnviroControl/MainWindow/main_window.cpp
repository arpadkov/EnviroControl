#include "MainWindow.h"
#include "ForecastData.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindowClass())
{
	ui->setupUi(this);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::onWeatherDataReady(const WFP::ForecastData& data)
{
	ui->_test_l->setText(data.toString() + " Hellooo");
}


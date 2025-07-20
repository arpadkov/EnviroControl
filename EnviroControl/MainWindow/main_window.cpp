#include "MainWindow.h"
#include "ErrorDetail.h"
#include "ForecastData.h"
#include "WeatherForecast.h"
#include "Logging.h"
#include "AutomationEngine.h"
#include "AutomationWidget.h"
#include "WeatherStation.h"

#include <QtCore/QThread>
#include <QtCore/QDir>
#include <QtCore/QProcess>

MainWindow::MainWindow(const Cfg::Config& cfg, QWidget* parent)
	: QMainWindow(parent), _cfg(cfg)
	, ui(new Ui::MainWindowClass())
{
	ui->setupUi(this);

	initWeatherForecastThread();
	initAutomationEngine();
	initWeatherStationThread();

	// TESTING PYTHON EXECUTION
	QString python_venv_path = _cfg.indoor_station_cfg.python_venv_path;
	QString python_interpreter_path;

	QString script_name = "main.py";
	QString sript_path = QCoreApplication::applicationDirPath() + QDir::separator() + script_name;

#ifdef Q_OS_WIN // If compiling for Windows
	python_interpreter_path = QDir(python_venv_path).absoluteFilePath("Scripts/python.exe");
#else // Assume Linux
	python_interpreter_path = QDir(python_venv_path).absoluteFilePath("bin/python"); // or "bin/python3"
#endif

	if (!QFile::exists(python_interpreter_path))
	{
		qWarning() << "Error: Python interpreter not found at:" << python_interpreter_path;
		return;
	}

	if (!QFile::exists(sript_path))
	{
		qWarning() << "Error: Python script not found at:" << sript_path;
		return;
	}

	QProcess* process = new QProcess(this);
	process->setWorkingDirectory(QFileInfo(sript_path).absoluteDir().path());
	process->start(python_interpreter_path, QStringList() << script_name);

	connect(process, &QProcess::readyReadStandardOutput, [=]()
		{
			QString output = process->readAllStandardOutput().trimmed();
			if (!output.isEmpty())
			{
				qDebug() << "Python STDOUT:" << output;
				ui->_python_test_l->setText(output);
			}
		});

	connect(process, &QProcess::readyReadStandardError, [=]()
		{
			QString error = process->readAllStandardError().trimmed();
			if (!error.isEmpty())
			{
				qWarning() << "Python STDERR:" << error;
			}
		});

	connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		[=](int exit_code, QProcess::ExitStatus exit_status)
		{
			if (exit_status == QProcess::NormalExit && exit_code == 0)
			{
				qDebug() << "Python script finished successfully.";
			}
			else
			{
				qWarning() << "Python script finished with error or abnormal exit. Exit code:" << exit_code;
				qWarning() << "Error string (QProcess):" << process->errorString();
			}
			process->deleteLater(); // Clean up the QProcess object
		});

	connect(process, &QProcess::errorOccurred, [=](QProcess::ProcessError error)
		{
			qWarning() << "QProcess Error occurred:" << error << process->errorString();
		});
}

MainWindow::~MainWindow()
{
	delete ui;

	// Clean up weatherforecast
	_weather_forecast_thread->exit();
	_weather_forecast_thread->wait();
}

void MainWindow::onWeatherForecastData(const WFP::ForecastData& data)
{
	ui->_test_l->setText(data.toString() + " Hellooo");
}

void MainWindow::onWeatherData(const WeatherData& data)
{
	ui->_weather_station_label->setText(data.toDebugString());
}

void MainWindow::initWeatherForecastThread()
{
	_weather_forecast_thread = new QThread();
	auto forecast = new WFP::WeatherForecast(_cfg.forecast_cfg);
	forecast->moveToThread(_weather_forecast_thread);

	// Start & finish signals
	QObject::connect(_weather_forecast_thread, &QThread::started, forecast, &WFP::WeatherForecast::startFetching);
	QObject::connect(_weather_forecast_thread, &QThread::finished, forecast, &QObject::deleteLater);

	QObject::connect(forecast, &WFP::WeatherForecast::forecastDataReady, [](const WFP::ForecastData& data)
		{
			qDebug(main_win_log) << "Weather forecast ready:" << data.toString();
		});
	QObject::connect(forecast, &WFP::WeatherForecast::forecastDataReady, this, &MainWindow::onWeatherForecastData);

	QObject::connect(forecast, &WFP::WeatherForecast::errorOccurred, [](const ErrorDetail& error)
		{
			qDebug(main_win_log) << "Error in weather forecast:" << error.getErrorMessage();
		});

	_weather_forecast_thread->start();
}

void MainWindow::initWeatherStationThread()
{
	_weather_station_thread = new QThread();
	auto weather_station = new WeatherStation(_cfg.weather_station_cfg);
	weather_station->moveToThread(_weather_station_thread);

	// Start & finish signals
	QObject::connect(_weather_station_thread, &QThread::started, weather_station, &WeatherStation::startReading);
	QObject::connect(_weather_station_thread, &QThread::finished, weather_station, &QObject::deleteLater);

	// Notify AutomationEngine when weather data is ready
	QObject::connect(weather_station, &WeatherStation::weatherDataReady, _automation_engine, &Automation::AutomationEngine::onWeatherStationData);


	QObject::connect(weather_station, &WeatherStation::weatherDataReady, this, &MainWindow::onWeatherData);
	QObject::connect(weather_station, &WeatherStation::errorOccurred, this, [this](const QString& error)
		{
			ui->_weather_station_label->setText(error);
		});

	_weather_station_thread->start();
}

void MainWindow::initAutomationEngine()
{
	_automation_engine = new Automation::AutomationEngine(_cfg.device_cfg_list, this);

	auto automation_widget = new Automation::AutomationWidget(_cfg.device_cfg_list, _automation_engine, this);
	ui->_manual_ctrl_layout->addWidget(automation_widget);

	_automation_engine->loadRules(_cfg.rules_cfg_relative_path);
}



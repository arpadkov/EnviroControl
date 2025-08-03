#include "IndoorStation.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QProcess>

IndoorStation::IndoorStation(const Cfg::IndoorStationConfig& cfg, QObject* parent)
	: QObject(parent), _cfg(cfg)
{
}

void IndoorStation::startReading()
{
	if (_dht_reader_process && _dht_reader_process->state() == QProcess::Running)
	{
		qDebug() << "(IndoorStation) PyDHT22 process is already running.";
		return;
	}

	// TODO: implement retrying mechanism if the process fails to start
	initPyDHT22Process();
}

void IndoorStation::stopReading()
{
	if (_dht_reader_process && _dht_reader_process->state() == QProcess::Running)
	{
		qDebug() << "(IndoorStation) Stopping PyDHT22 process.";
		_dht_reader_process->terminate(); // Gracefully terminate the process
		_dht_reader_process->waitForFinished(); // Wait for it to finish
		_dht_reader_process->deleteLater(); // Clean up the QProcess object
	}
	else
	{
		qDebug() << "(IndoorStation) No running PyDHT22 process to stop.";
	}
}

void IndoorStation::initPyDHT22Process()
{
	QString python_venv_path = _cfg.python_venv_path;
	QString python_interpreter_path;

	QString script_name = "main.py";
	QString script_path = QCoreApplication::applicationDirPath() + QDir::separator() + script_name;

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

	if (!QFile::exists(script_path))
	{
		qWarning() << "Error: Python script not found at:" << script_path;
		return;
	}

	_dht_reader_process = new QProcess(this);
	_dht_reader_process->setWorkingDirectory(QFileInfo(script_path).absoluteDir().path());

	connect(_dht_reader_process, &QProcess::readyReadStandardOutput, [this]()
		{
			QString output = _dht_reader_process->readAllStandardOutput().trimmed();
			if (!output.isEmpty())
			{
				if (auto data = parseData(output))
					Q_EMIT indoorDataReady(*data);
			}
		});

	connect(_dht_reader_process, &QProcess::readyReadStandardError, [this]()
		{
			QString error = _dht_reader_process->readAllStandardError().trimmed();
			if (!error.isEmpty())
			{
				// DHT22 sucks, and this will happen frequently -> only log an error
				qWarning() << "(IndoorStation) received error from PyDHT22 process: " << error;
			}
		});

	connect(_dht_reader_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		[this](int exit_code, QProcess::ExitStatus exit_status)
		{
			if (exit_status == QProcess::NormalExit && exit_code == 0)
			{
				qDebug() << "(IndoorStation) PyDHT22 script finished successfully.";
			}
			else
			{
				qWarning() << "(IndoorStation) PyDHT22 script finished with error or abnormal exit. Exit code:" << exit_code;
				qWarning() << "Error string (QProcess):" << _dht_reader_process->errorString();
			}
			_dht_reader_process->deleteLater(); // Clean up the QProcess object
		});

	connect(_dht_reader_process, &QProcess::errorOccurred, [this](QProcess::ProcessError error)
		{
			qWarning() << "(IndoorStation) PyDHT22 Error occurred:" << error << _dht_reader_process->errorString();
		});

	QStringList args;
	args << script_name;
	args << QString::number(_cfg.polling_interval_sec);
	args << QString::number(_cfg.data_gpio_pin);

	qDebug() << "(IndoorStation) Starting PyDHT22 process with interpreter:" << python_interpreter_path << " " << args.join(", ");
	_dht_reader_process->start(python_interpreter_path, args);
}

std::optional<IndoorData> IndoorStation::parseData(const QString& data_str) const
{
	static const QString DATA_PREFIX = "DATA:";

	// If the line is not a data message, it's a log message.
	if (!data_str.startsWith(DATA_PREFIX))
	{
		// Log the message using qDebug()
		qDebug() << "PyDHT22 Log:" << data_str;
		return std::nullopt; // Return an empty optional
	}

	QString values_str = data_str.mid(DATA_PREFIX.length() + 1);
	QStringList values = values_str.split(',');

	if (values.size() != 2)
	{
		qWarning() << "PyDHT22 Parsing Error: Expected 2 values, but got" << values.size();
		return std::nullopt;
	}

	bool ok_temp_c, ok_humidity;
	double temp_celsius = values[0].toDouble(&ok_temp_c);
	double humidity = values[1].toDouble(&ok_humidity);

	if (!ok_temp_c || !ok_humidity)
	{
		qWarning() << "PyDHT22 Parsing Error: Failed to convert one or more values to double. Input:" << data_str;
		return std::nullopt;
	}

	IndoorData data{ temp_celsius, humidity, QDateTime::currentDateTime()};
	return data;
}

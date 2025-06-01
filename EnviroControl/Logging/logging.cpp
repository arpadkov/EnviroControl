#include "logging.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QLoggingCategory>
#include <QFileInfo>
#include <QDir>

#include <QStandardPaths>

// For OutputDebugString - these headers are Windows-specific
#ifdef Q_OS_WIN
#include <windows.h>
#endif

Q_LOGGING_CATEGORY(appLog, "envirocontrol")

namespace
{
QFile logFile;
}

namespace Log
{

void setLogFile(const QString& filePath)
{
	if (logFile.isOpen())
		logFile.close();

	logFile.setFileName(filePath);
	if (logFile.open(QIODevice::Append | QIODevice::Text))
	{
		auto handler = [](QtMsgType type, const QMessageLogContext& context, const QString& msg)
			{
				QString formatted_msg;
				QTextStream log_stream(&formatted_msg);
				log_stream << QDateTime::currentDateTime().toString(Qt::ISODate) << " ";
				switch (type)
				{
				case QtDebugMsg:    log_stream << "[DEBUG] "; break;
				case QtInfoMsg:     log_stream << "[INFO] "; break;
				case QtWarningMsg:  log_stream << "[WARN] "; break;
				case QtCriticalMsg: log_stream << "[CRIT] "; break;
				case QtFatalMsg:    log_stream << "[FATAL] "; break;
				}
				log_stream << msg << "\n";

				QTextStream file_out(&logFile);
				file_out << formatted_msg;
				file_out.flush();

				// Write to Visual Studio Output Console (Windows only, Debug mode) ---
#ifdef Q_OS_WIN
#ifdef _DEBUG
				OutputDebugStringA(formatted_msg.toLocal8Bit().constData());
#endif // _DEBUG
#endif // Q_OS_WIN

			};
		qInstallMessageHandler(handler);
	}
}

void init()
{
	// Determine a suitable log file path (e.g., in the user's writable location)
	QString log_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + "EnviroControl";
	QDir().mkpath(log_dir); // Ensure the directory exists
	QString log_file_path = log_dir + QDir::separator() + "envirocontrol.log";
	setLogFile(log_file_path);
}

}
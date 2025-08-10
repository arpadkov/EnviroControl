#pragma once

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(app_log)
Q_DECLARE_LOGGING_CATEGORY(main_win_log)
Q_DECLARE_LOGGING_CATEGORY(device_log)

namespace Log
{
class Logger
{
public:
	static Logger* instance();

private:
	Logger();
	~Logger();
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

public:
	QFile info_log_file;
	QFile debug_log_file;
};

void init();
}
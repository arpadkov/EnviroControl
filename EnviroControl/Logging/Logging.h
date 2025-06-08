#pragma once

#include <QString>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(app_log)
Q_DECLARE_LOGGING_CATEGORY(main_win_log)
Q_DECLARE_LOGGING_CATEGORY(device_log)

namespace Log
{
void init();
}
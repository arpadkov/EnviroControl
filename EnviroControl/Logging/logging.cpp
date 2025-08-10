#include "Logging.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QLoggingCategory>
#include <QFileInfo>
#include <QDir>

#include <QStandardPaths>

// Platform-specific headers for stack trace
#ifdef Q_OS_WIN
#include <windows.h>
#include <DbgHelp.h> // Requires linking with DbgHelp.lib
#pragma comment(lib, "Dbghelp.lib") // Link Dbghelp.lib directly in code (MSVC)
#endif

#ifdef Q_OS_UNIX // For Linux/macOS (check your compiler/system for availability)
#include <execinfo.h> // For backtrace, backtrace_symbols
#include <cxxabi.h>   // For abi::__cxa_demangle (C++ symbol demangling)
#endif

// For OutputDebugString - these headers are Windows-specific
#ifdef Q_OS_WIN
#include <windows.h>
#endif

Q_LOGGING_CATEGORY(app_log, "envirocontrol")
Q_LOGGING_CATEGORY(main_win_log, "main_window")
Q_LOGGING_CATEGORY(device_log, "device")



namespace Log
{

// AI code
QString getStackTrace()
{
  QString stackTrace;
  QTextStream ss(&stackTrace);

#ifdef Q_OS_WIN
  const int MAX_FRAMES = 62;
  const int SKIP_INITIAL_FRAMES = 3; // Skip frames related to the logging itself (getStackTrace, lambda invokers)
  const int MAX_QT_SYSTEM_FRAMES_TO_SHOW = 5; // Show up to this many Qt/System frames if they are between app frames
  // Or, if we hit this many non-app frames consecutively, stop.

  void* stack[MAX_FRAMES];
  USHORT frames;
  SYMBOL_INFO* symbol;
  IMAGEHLP_LINE64 line;
  IMAGEHLP_MODULE64 moduleInfo;

  HANDLE process = GetCurrentProcess();
  SymInitialize(process, NULL, TRUE);

  frames = CaptureStackBackTrace(0, MAX_FRAMES, stack, NULL);
  symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
  symbol->MaxNameLen = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

  line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
  moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

  // Get the name of your application's executable to filter out internal frames
  // This is more robust than hardcoding "EnviroControl"
  TCHAR moduleFilePath[MAX_PATH];
  GetModuleFileName(NULL, moduleFilePath, MAX_PATH);
  QFileInfo appFileInfo(QString::fromWCharArray(moduleFilePath)); // Assuming WCHAR, adjust if your project is ANSI
  QString appModuleName = appFileInfo.baseName(); // e.g., "EnviroControl"

  int qtSystemFrameCount = 0; // Consecutive Qt/System frames

  for (USHORT i = SKIP_INITIAL_FRAMES; i < frames; i++) // Start from SKIP_INITIAL_FRAMES
  {
    DWORD64 address = (DWORD64)(stack[i]);
    DWORD64 displacement = 0;
    DWORD dwLineDisp;

    bool gotSymbol = SymFromAddr(process, address, &displacement, symbol);
    bool gotLine = SymGetLineFromAddr64(process, address, &dwLineDisp, &line);
    bool gotModule = SymGetModuleInfo64(process, address, &moduleInfo);

    QString currentModuleName = "UNKNOWN_MODULE";
    if (gotModule)
    {
      currentModuleName = QString::fromLocal8Bit(moduleInfo.ModuleName);
    }

    // Check if this frame belongs to your application
    bool isAppFrame = currentModuleName.compare(appModuleName, Qt::CaseInsensitive) == 0;

    if (!isAppFrame)
    {
      qtSystemFrameCount++;
      if (qtSystemFrameCount > MAX_QT_SYSTEM_FRAMES_TO_SHOW)
      {
        // If we've seen too many consecutive non-app frames, assume we're deep in system code
        ss << "  ... (further frames likely in Qt/System libraries) ...\n";
        break; // Stop processing further frames
      }
    }
    else
    {
      // Reset count if we hit an application frame
      qtSystemFrameCount = 0;
    }

    ss << QString("  %1: ").arg(i, 2, 10, QChar(' ')); // Frame number

    ss << currentModuleName;
    ss << "!";

    if (gotSymbol)
    {
      ss << QString("%1 + 0x%2").arg(QString::fromLocal8Bit(symbol->Name)).arg(displacement, QT_POINTER_SIZE, 16, QChar('0'));
    }
    else
    {
      ss << QString("0x%1 (Unknown Symbol)").arg(address, QT_POINTER_SIZE * 2, 16, QChar('0'));
    }

    if (gotLine)
    {
      ss << QString(" [%1:%2]").arg(QString::fromLocal8Bit(line.FileName)).arg(line.LineNumber);
    }

    ss << "\n"; // End line
  }

  free(symbol);
  SymCleanup(process);
#elif defined(Q_OS_UNIX)
  // Linux/macOS specific stack trace - unchanged, as filtering might be less necessary or done differently
  const int MAX_FRAMES = 50;
  const int SKIP_INITIAL_FRAMES = 1; // You might need to adjust this for Unix too, 1 or 2 is common.
  void* callstack[MAX_FRAMES];
  int frames = backtrace(callstack, MAX_FRAMES);
  char** strs = backtrace_symbols(callstack, frames);

  // Get the name of your executable on Unix-like systems for filtering
  // This is a bit more involved, often read from /proc/self/exe or argv[0]
  // For simplicity, we'll assume the primary executable is the one you care about.
  // A more robust solution might use dladdr to inspect loaded modules.
  // For now, no specific filtering by app module name here, only initial skip.

  for (int i = SKIP_INITIAL_FRAMES; i < frames; ++i)
  { // Start from SKIP_INITIAL_FRAMES
    QString line = QString::fromUtf8(strs[i]);

    int status;
    char* demangled_name = abi::__cxa_demangle(line.toUtf8().constData(), nullptr, nullptr, &status);
    if (status == 0)
    {
      ss << QString("  %1: %2\n").arg(i, 2, 10, QChar(' ')).arg(demangled_name);
      free(demangled_name);
    }
    else
    {
      ss << QString("  %1: %2\n").arg(i, 2, 10, QChar(' ')).arg(line);
    }
  }
  free(strs);
#else
  ss << "  Stack trace not available on this platform.\n";
#endif

  return stackTrace;
}

static Logger* logger_instance = nullptr;

Logger* Logger::instance()
{
  if (!logger_instance)
  {
    logger_instance = new Logger();
    qDebug() << "Logger instance created.";
  }
	return logger_instance;
}

Logger::Logger()
{
  // Determine a suitable log file path (e.g., in the user's writable location)
  QString log_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + "EnviroControl";
  QDir().mkpath(log_dir); // Ensure the directory exists
  qDebug() << "Log directory:" << log_dir;

  // INFO file
  QString info_log_file_path = log_dir + QDir::separator() + "envirocontrol_info.log";
  info_log_file.setFileName(info_log_file_path);
  if (!info_log_file.open(QIODevice::Append | QIODevice::Text))
    qWarning() << "Failed to open info log file:" << info_log_file_path;

  // DEBUG file
  QString debug_log_file_path = log_dir + QDir::separator() + "envirocontrol_debug.log";
  debug_log_file.setFileName(debug_log_file_path);
  if (!debug_log_file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    qWarning() << "Failed to open debug log file:" << debug_log_file_path;
}

Logger::~Logger()
{
  if (Logger::instance()->info_log_file.isOpen())
		Logger::instance()->info_log_file.close();
	if (Logger::instance()->debug_log_file.isOpen())
    Logger::instance()->debug_log_file.close();
  logger_instance = nullptr; // Clear the instance pointer
	qDebug() << "Logger instance destroyed and files closed.";
}

void init()
{
	logger_instance = Logger::instance();

	qSetMessagePattern(
		"%{time system} %{type} %{category}: %{message}"
		"%{if-warning}\n  Backtrace:\n  %{backtrace depth=10 separator=\"\n  \"}%{endif}"
	);

	auto handler = [](QtMsgType type, const QMessageLogContext& context, const QString& msg)
		{
      if (!Logger::instance())
      {
        qWarning() << "Logger instance is not initialized!";
        return;
			}

      QString formatted_msg;
      QTextStream log_stream(&formatted_msg);

      // 1. Timestamp
      log_stream << QDateTime::currentDateTime().toString(Qt::ISODate) << " ";

      // 2. Message Type
      switch (type)
      {
      case QtDebugMsg:    log_stream << "[DEBUG] "; break;
      case QtInfoMsg:     log_stream << "[INFO] "; break;
      case QtWarningMsg:  log_stream << "[WARN] "; break;
      case QtCriticalMsg: log_stream << "[CRIT] "; break;
      case QtFatalMsg:    log_stream << "[FATAL] "; break;
      }

      // 3. Optional: Add Category (similar to %{category} in qSetMessagePattern)
      // Check if context has a category and it's not empty
      if (context.category && *context.category != '\0')
      {
        log_stream << QString("[%1] ").arg(context.category);
      }

      // 4. The actual message
      log_stream << msg;

      // 5. Conditional: Add Stack Trace for Warnings
      if (type == QtWarningMsg)
      {
        log_stream << "\n  --- Backtrace ---\n" << getStackTrace();
      }

      // 6. Final Newline
      log_stream << "\n";

      // Write to info log only for Info, Warn, Critical, Fatal
			auto& info_log_file = Logger::instance()->info_log_file;
      if (type >= QtInfoMsg && info_log_file.isOpen())
      {
        QTextStream file_out(&info_log_file);
        file_out << formatted_msg;
        file_out.flush();
      }

      // Write all message types to the debug log
			auto& debug_log_file = Logger::instance()->debug_log_file;
      if (debug_log_file.isOpen())
      {
        QTextStream debug_file_out(&debug_log_file);
        debug_file_out << formatted_msg;
        debug_file_out.flush();
      }

      // Write to Visual Studio Output Console (Windows only, Debug mode) ---
#ifdef Q_OS_WIN
#ifdef _DEBUG
      OutputDebugStringA(formatted_msg.toLocal8Bit().constData());
#endif // _DEBUG
#endif // Q_OS_WIN

		}; // handler

	qInstallMessageHandler(handler);
}

}
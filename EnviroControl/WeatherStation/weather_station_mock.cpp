#include "WeatherStationMock.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

namespace
{
QString getMockFilePath(const Cfg::WeatherStationConfig& cfg)
{
  QFileInfo originalFileInfo(cfg.log_file_path);
  QString directoryPath = originalFileInfo.absolutePath();
  static const QString mock_file_name = "mock_weather_data.json";

  return QDir(directoryPath).filePath(mock_file_name);
}
}

WeatherStationMock::WeatherStationMock(const Cfg::WeatherStationConfig& cfg, QObject* parent)
	: IWeatherStation(cfg, parent), _mock_file_path(getMockFilePath(cfg))
{
}

WeatherStationMock::~WeatherStationMock()
{
}

void WeatherStationMock::startReading()
{
  qDebug() << "WeatherStationMock::startReading()";

  // 1. Initialize timer if not already done
  if (!_read_timer)
  {
    _read_timer = new QTimer(this); // Parent the timer to this WeatherStationMock object
    connect(_read_timer, &QTimer::timeout, this, &WeatherStationMock::readAndEmitLatestMockData);
    qDebug() << "WeatherStationMock: QTimer created and connected.";
  }

  // 2. Start the timer
  if (!_read_timer->isActive())
  {
    int interval_ms = 5000;

    _read_timer->start(interval_ms);
    qInfo() << QString("WeatherStationMock: Started polling mock data file %1 with interval %2 ms.").arg(_mock_file_path).arg(interval_ms);
  }

  // Emit the first data point immediately upon starting
  readAndEmitLatestMockData();
}

void WeatherStationMock::stopReading()
{
  if (_read_timer && _read_timer->isActive())
  {
    _read_timer->stop();
    qDebug() << "WeatherStationMock: Stopped polling mock data.";
  }
}

void WeatherStationMock::readAndEmitLatestMockData()
{
  std::vector<WeatherData> weather_data_list = WeatherDataLogger::parseWeatherDataFromFile(_mock_file_path);

  if (weather_data_list.empty())
  {
    qWarning() << "WeatherStationMock: Mock data file is empty or unreadable:" << _mock_file_path;
    Q_EMIT errorOccurred(QString("Mock data file empty or unreadable: %1").arg(_mock_file_path));
    // No need to stop timer here, continue polling in case file becomes available/populated
    return;
  }

  // Get the last element (most recent data entry)
  WeatherData latest_data = weather_data_list.back();

  // Update timestamp to current time for realism, even if the file has an older timestamp
  latest_data.timestamp = QDateTime::currentDateTime();

  Q_EMIT weatherDataReady(latest_data);
}
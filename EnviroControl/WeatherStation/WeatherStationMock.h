#pragma once

#include "WeatherStation.h"

class WeatherStationMock : public IWeatherStation
{
	Q_OBJECT

public:
	WeatherStationMock(const Cfg::WeatherStationConfig& cfg, QObject* parent = nullptr);
	~WeatherStationMock();

public Q_SLOTS:
	void startReading() override;
	void stopReading() override;

private Q_SLOTS:
	void readAndEmitLatestMockData();

private:
	QTimer* _read_timer = nullptr;
	const QString _mock_file_path;
};
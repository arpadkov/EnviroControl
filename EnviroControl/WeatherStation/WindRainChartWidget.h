#pragma once

#include "WeatherData.h"

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>

class QChart;
class QChartView;
class QLineSeries;
class QAreaSeries;

class WindRainChartWidget : public QWidget
{
  Q_OBJECT

public:
  explicit WindRainChartWidget(int history_length_sec, QWidget* parent = nullptr);
  ~WindRainChartWidget();

public Q_SLOTS:
  void onWeatherData(const WeatherData& data);

private:
  void setupChart();
  void updateCharts();

  QPointer<QChart> _chart;
  QPointer<QChartView> _chart_view;
  QPointer<QLineSeries> _wind_series;

  // QAreaSeries for rain
  QAreaSeries* _rain_series;
  QLineSeries* _rain_upper_series;
  QLineSeries* _rain_lower_series;

  std::vector<WeatherData> _weather_history;
  int _history_length_sec;
};
#pragma once

#include "WeatherHistoryWidgetBase.h"

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>

class QChart;
class QChartView;
class QLineSeries;
class QAreaSeries;

class WindRainChartWidget : public WeatherHistoryWidgetBase
{
  Q_OBJECT

public:
  explicit WindRainChartWidget(int history_length_sec, QWidget* parent = nullptr);
  ~WindRainChartWidget();

private:
  void setupChart() override;
  void updateCharts() override;

  QPointer<QLineSeries> _wind_series;

  // QAreaSeries for rain
  QAreaSeries* _rain_series;
  QLineSeries* _rain_upper_series;
  QLineSeries* _rain_lower_series;
};
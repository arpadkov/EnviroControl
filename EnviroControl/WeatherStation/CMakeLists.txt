# WeatherStation

qt_add_library(WeatherStation STATIC
    WeatherStation.h
    weather_station.cpp
    weather_station_if.cpp
    WeatherStationMock.h
    weather_station_mock.cpp
    WeatherStationWidget.h
    weather_station_widget.cpp
    WindWheelWidget.h
    wind_wheel_widget.cpp
    WeatherHistoryWidget.h
    weather_history_widget.cpp
    WindRainChartWidget.h
    wind_rain_chart_widget.cpp
    WeatherDataLogger.h
    WeatherDataFormat.h
    WeatherData.h
    weather_data_logger.cpp
    SunPlotWidget.h
    sun_plot_widget.cpp
    ThermometerWidget.h
    thermometer_widget.cpp
    IndoorStation.h
    indoor_station.cpp
    IndoorStationWidget.h
    indoor_station_widget.cpp
)

# Specify include directories for this library.
# The 'PRIVATE' means these are for SensorsLib's internal use.
# The 'PUBLIC' or 'INTERFACE' would be if other targets need to include headers from this folder.
# 'PROJECT_SOURCE_DIR' here refers to the root of the 'Sensors' module.
target_include_directories(WeatherStation
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # Allows other targets to include this
)

target_link_libraries(WeatherStation PRIVATE
    Logging    
    ErrorDetail
    Config

    Qt6::Charts
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickWidgets
    Qt6::Svg
    Qt6::SerialPort
)
# WeatherForecastProvider

qt_add_library(WeatherForecastProvider STATIC
    WeatherForecast.h
    weather_forecast.cpp
    ForecastData.h
    forecast_data.cpp
)

target_include_directories(WeatherForecastProvider
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # Allows other targets to include this
)

target_link_libraries(WeatherForecastProvider PRIVATE
    Logging
    ErrorDetail
    Qt6::Core
    Qt6::Network
)
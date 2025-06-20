# WeatherStation

qt_add_library(WeatherStation STATIC
    WeatherStation.h
    weather_station.cpp
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
    ErrorDetail
    Config
    Qt6::Core
)
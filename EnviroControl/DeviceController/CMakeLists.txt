# DeviceController

qt_add_library(DeviceController STATIC
    DeviceDriver.h
    device_driver.cpp
    DeviceState.h
    test_device_driver.cpp
    DeviceStateManager.h
    device_state_manager.cpp
)

target_include_directories(DeviceController
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # Allows other targets to include this
)

target_link_libraries(DeviceController PRIVATE
    Logging
    ErrorDetail
    WeatherForecastProvider
    WeatherStation
    Config
    Qt6::Core
)
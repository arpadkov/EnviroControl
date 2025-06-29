# DeviceController

# Conditional dependencies for libgpiod (Linux only)
if(NOT WIN32)
    find_package(PkgConfig REQUIRED)
    # Check for libgpiodcxx, which is the C++ binding for libgpiod
    pkg_check_modules(GPIOD REQUIRED libgpiodcxx)
    message(STATUS "Building on Linux. Found libgpiodcxx: ${GPIOD_VERSION}")
else()
    message(STATUS "Building on Windows. Skipping libgpiod dependencies.")
endif()

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

# Link libgpiod only on Linux
if(NOT WIN32)
    target_link_libraries(DeviceController PRIVATE ${GPIOD_LDFLAGS})
    target_include_directories(DeviceController PRIVATE ${GPIOD_CFLAGS_INCLUDE_DIRS})
endif()

target_link_libraries(DeviceController PRIVATE
    Logging
    ErrorDetail
    WeatherForecastProvider
    WeatherStation
    Config
    Qt6::Core
)
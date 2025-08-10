# MainWindow

find_package(Qt6 COMPONENTS Widgets REQUIRED)
qt_add_library(MainWindow STATIC
    MainWindow.ui
    MainWindow.h
    main_window.cpp
    ErrorDetailsWidget.h
    error_details_widget.cpp
)

target_include_directories(MainWindow PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # Allows other targets to include this
)

target_link_libraries(MainWindow PRIVATE
    Logging
    ErrorDetail
    WeatherForecastProvider
    WeatherStation
    Config
    DeviceController
    AutomationEngine

    Qt6::Core
    Qt6::Gui
    Qt6::Svg
    Qt6::Widgets
)
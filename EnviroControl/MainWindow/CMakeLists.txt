# MainWindow

qt_add_library(MainWindow STATIC
    MainWindow.ui
    MainWindow.h
    main_window.cpp
)

target_include_directories(MainWindow
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # Allows other targets to include this
)

target_link_libraries(MainWindow PRIVATE
    Logging
    WeatherForecastProvider
    WeatherStation
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
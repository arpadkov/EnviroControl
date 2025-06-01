# MainWindow

find_package(Qt6 COMPONENTS Widgets REQUIRED)
qt_add_library(MainWindow STATIC
    MainWindow.ui
    MainWindow.h
    main_window.cpp
)
message(STATUS "MAIN WINDOW LIB ADDED")


target_include_directories(MainWindow
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # Allows other targets to include this
)

target_include_directories(MainWindow INTERFACE
    # This retrieves the automatically generated include directory path for the 'MainWindow' target
    $<TARGET_PROPERTY:MainWindow,AUTOGEN_INCLUDE_DIR>
)

target_link_libraries(MainWindow PRIVATE
    Logging
    WeatherForecastProvider
    WeatherStation
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
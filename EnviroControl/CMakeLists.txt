cmake_minimum_required(VERSION 3.16)
project(EnviroControl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Network
        SerialPort
)
qt_standard_project_setup()

add_subdirectory(Logging)
add_subdirectory(ErrorDetail)
add_subdirectory(WeatherForecastProvider)
add_subdirectory(WeatherStation)
add_subdirectory(MainWindow)

set(PROJECT_SOURCES
    main.cpp
)

message(STATUS "BUILDING EnviroControl with Qt version: ${Qt6_VERSION}")

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    # This retrieves the automatically generated include directory path for the 'MainWindow' target
    $<TARGET_PROPERTY:MainWindow,AUTOGEN_INCLUDE_DIR>
)

# --- PLATFORM-SPECIFIC SETTINGS ---
# Set WIN32_EXECUTABLE property ONLY for Windows builds.
# On Linux (Raspberry Pi), this property is not applicable and would cause a warning/error.
if(WIN32)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            WIN32_EXECUTABLE TRUE
    )
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::SerialPort

        Logging
        ErrorDetail
        WeatherForecastProvider
        MainWindow
        WeatherStation
)


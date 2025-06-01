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

# Define a macro to add a subdirectory and optionally its generated Qt UI include path
# Usage: add_component(ComponentName [HAS_UI])
#   - COMPONENT_NAME: The name of the subdirectory (and typically the target within it)
#   - HAS_UI: Optional argument. If present, it indicates the component has UI files
#             and its generated _autogen/include path should be added to the main executable for the pi compiler.
macro(add_component COMPONENT_NAME)
    # Add the subdirectory to the build
    add_subdirectory(${COMPONENT_NAME})

    foreach(arg IN ITEMS ${ARGN})
        if (arg STREQUAL "HAS_UI")
            # Add the generated include directory to the main executable
            target_include_directories(${PROJECT_NAME} PRIVATE
                "${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT_NAME}/${COMPONENT_NAME}_autogen/include"
            )
            # Break after finding HAS_UI to avoid adding it multiple times if other args are present
            break()
        endif()
    endforeach()
endmacro()

set(PROJECT_SOURCES
    main.cpp
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

add_component(Logging)
add_component(Config)
add_component(ErrorDetail)
add_component(WeatherForecastProvider)
add_component(WeatherStation)
add_component(MainWindow HAS_UI)

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


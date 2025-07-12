cmake_minimum_required(VERSION 3.16)
project(EnviroControl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This ensures all executables (main app, tests) go to a predictable location.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release")

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Network
        SerialPort
        Svg
)
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

# Resources
qt_add_resources (EnviroControlResources
    "Resources/resources.qrc"
)

# Fetch and enable google test
enable_testing()
include(FetchContent)
FetchContent_Declare(
    GoogleTest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
    URL_HASH SHA256=24564e3b712d3eb30ac9a85d92f7d720f60cc0173730ac166f27dda7fed76cb2 # Add SHA256 hash for security/integrity
)

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
    ${EnviroControlResources}
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

add_component(Logging)
add_component(Config)
add_component(ErrorDetail)
add_component(WeatherForecastProvider)
add_component(WeatherStation)
add_component(DeviceController)
add_component(MainWindow HAS_UI)
add_component(AutomationEngine HAS_UI)

# --- PLATFORM-SPECIFIC SETTINGS ---
# Set WIN32_EXECUTABLE property ONLY for Windows builds.
# On Linux (Raspberry Pi), this property is not applicable and would cause a warning/error.
if(WIN32)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            WIN32_EXECUTABLE TRUE
    )
endif()

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::SerialPort
        Qt6::Svg

        Logging
        Config
        ErrorDetail
        WeatherForecastProvider
        WeatherStation
        AutomationEngine
        DeviceController
        MainWindow
)

# ---- Application Deployment ----
# Get paths to Qt's installation directories
# QT_CORE_LOCATION will be something like C:/Qt/6.8.0/msvc2019_64/bin/Qt6Cored.dll or /opt/Qt/6.8.0/gcc_64/lib/libQt6Core.so
get_target_property(QT_CORE_LOCATION Qt6::Core LOCATION)
get_filename_component(QT_BIN_DIR ${QT_CORE_LOCATION} DIRECTORY) # e.g., C:/Qt/6.8.0/msvc2019_64/bin or /opt/Qt/6.8.0/gcc_64/lib

# Try to get the official Qt plugin directory property. This is the most reliable.
get_target_property(QT_PLUGINS_ROOT_DIR Qt6::Core QT_PLUGINS_DIR)
if (NOT QT_PLUGINS_ROOT_DIR)
    # Fallback if QT_PLUGINS_DIR property is not set (less common for Qt6, but good for robustness)
    # This assumes plugins are typically in a 'plugins' folder one level up from the 'bin' directory.
    get_filename_component(QT_INSTALL_PREFIX ${QT_BIN_DIR} DIRECTORY)
    set(QT_PLUGINS_ROOT_DIR "${QT_INSTALL_PREFIX}/plugins")
endif()

# Define the destination directory for the deployed files (where your .exe or executable is)
# $<TARGET_FILE_DIR:${PROJECT_NAME}> ensures it's the correct Debug/Release/etc. subfolder
set(APP_DEPLOY_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

# Define common DLL/SO suffix (e.g., .dll, .so)
set(QT_LIB_SUFFIX "$<IF:$<BOOL:${WIN32}>,.dll,.so>")

# List of Qt modules to copy (core libraries)
set(QT_CORE_LIBS
    Qt6Core
    Qt6Gui
    Qt6Widgets
    Qt6Network
    Qt6SerialPort
    Qt6Svg
)

# Construct the list of full paths for Qt core libraries
set(QT_CORE_LIB_FULL_PATHS "")
foreach(lib IN ITEMS ${QT_CORE_LIBS})
    list(APPEND QT_CORE_LIB_FULL_PATHS "${QT_BIN_DIR}/${lib}${QT_LIB_SUFFIX}")
endforeach()

# Add a custom command to copy all necessary Qt DLLs/SOs
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "--- Manually Deploying Qt Runtime Dependencies ---"
    # Ensure destination exists
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APP_DEPLOY_DIR}"
    # Use the pre-constructed list of full paths directly
    COMMAND ${CMAKE_COMMAND} -E copy ${QT_CORE_LIB_FULL_PATHS} "${APP_DEPLOY_DIR}"
    COMMENT "Copying essential Qt core libraries."
)

# Copy 'platforms' plugin directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_PLUGINS_ROOT_DIR}/platforms"
            "${APP_DEPLOY_DIR}/platforms"
    COMMENT "Copying Qt platform plugins."
)

# Copy 'imageformats' plugin directory (includes SVG plugin)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_PLUGINS_ROOT_DIR}/imageformats"
            "${APP_DEPLOY_DIR}/imageformats"
    COMMENT "Copying Qt imageformats plugins (including SVG)."
)

# Ensure GoogleTest components are available for the subdirectories
FetchContent_MakeAvailable(GoogleTest)




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
set(QT_PLUGINS_ROOT_DIR "") # Initialize to empty
get_target_property(QT_PLUGINS_ROOT_DIR Qt6::Core QT_PLUGINS_DIR)
if (QT_PLUGINS_ROOT_DIR_PROP)
    set(QT_PLUGINS_ROOT_DIR "${QT_PLUGINS_ROOT_DIR_PROP}")
    message(STATUS "Resolved Qt Plugin Root Directory using QT_PLUGINS_DIR property: ${QT_PLUGINS_ROOT_DIR}")
else()
    message(STATUS "Qt6::Core QT_PLUGINS_DIR property not directly available, attempting platform-specific fallback.")

    # Determine the root of the Qt kit installation
    # This takes QT_BIN_DIR (e.g., C:/Qt/6.8.3/msvc2022_64/bin) and gets its parent (e.g., C:/Qt/6.8.3/msvc2022_64)
    get_filename_component(QT_KIT_ROOT_DIR ${QT_BIN_DIR} DIRECTORY)

    if (WIN32)
        # --- Fallback for Windows ---
        # Based on your feedback, plugins are directly under QT_KIT_ROOT_DIR + /plugins
        if (EXISTS "${QT_KIT_ROOT_DIR}/plugins")
            set(QT_PLUGINS_ROOT_DIR "${QT_KIT_ROOT_DIR}/plugins")
        endif()
    else() # Assume UNIX/Linux for Raspberry Pi
        # --- Fallback for Linux/UNIX ---
        # On Linux, plugins can be in various locations.
        # We also need QT_INSTALL_PREFIX for some common Linux paths.

        # Determine the root of the Qt kit installation (this line should already be there)
        get_filename_component(QT_KIT_ROOT_DIR ${QT_BIN_DIR} DIRECTORY)
        # Temporary variable for QT_INSTALL_PREFIX for Linux path checks (this line should already be there)
        get_target_property(QT_INSTALL_PREFIX_TEMP Qt6::Core QT_INSTALL_PREFIX)
        
        # DEBUG LINES for Linux: Please provide the output of these
        message(STATUS "DEBUG LINUX: QT_CORE_LOCATION is: ${QT_CORE_LOCATION}")
        message(STATUS "DEBUG LINUX: QT_BIN_DIR is: ${QT_BIN_DIR}")
        message(STATUS "DEBUG LINUX: QT_KIT_ROOT_DIR is: ${QT_KIT_ROOT_DIR}")
        message(STATUS "DEBUG LINUX: QT_INSTALL_PREFIX_TEMP is: ${QT_INSTALL_PREFIX_TEMP}")

        if (EXISTS "${QT_KIT_ROOT_DIR}/plugins") # Common for some custom Qt builds or certain Linux installs
            set(QT_PLUGINS_ROOT_DIR "${QT_KIT_ROOT_DIR}/plugins")
        elseif (EXISTS "${QT_KIT_ROOT_DIR}/qt6/plugins") # Common on Debian/Ubuntu (e.g. /usr/lib/aarch64-linux-gnu/qt6/plugins)
            set(QT_PLUGINS_ROOT_DIR "${QT_KIT_ROOT_DIR}/qt6/plugins")
        elseif (EXISTS "${QT_INSTALL_PREFIX_TEMP}/lib/qt6/plugins") # Another common Linux structure (e.g., /usr/lib/qt6/plugins)
            set(QT_PLUGINS_ROOT_DIR "${QT_INSTALL_PREFIX_TEMP}/lib/qt6/plugins")
        elseif (EXISTS "${QT_INSTALL_PREFIX_TEMP}/lib/aarch64-linux-gnu/qt6/plugins") # Specific to arm64 on some systems
             set(QT_PLUGINS_ROOT_DIR "${QT_INSTALL_PREFIX_TEMP}/lib/aarch64-linux-gnu/qt6/plugins")
        endif()
    endif()
endif()

# Important: Add a check and a message to see what it resolved to
if (NOT QT_PLUGINS_ROOT_DIR OR NOT EXISTS "${QT_PLUGINS_ROOT_DIR}/platforms")
    message(FATAL_ERROR "Could not determine Qt plugin directory or '${QT_PLUGINS_ROOT_DIR}/platforms' does not exist. Please check your Qt installation paths. QT_PLUGINS_ROOT_DIR found as: '${QT_PLUGINS_ROOT_DIR}'")
else()
    message(STATUS "Resolved Qt Plugin Root Directory to: ${QT_PLUGINS_ROOT_DIR}")
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
foreach(lib_target IN ITEMS
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
    Qt6::Svg
)
    get_target_property(lib_location ${lib_target} LOCATION)
    if(lib_location)
        list(APPEND QT_CORE_LIB_FULL_PATHS "${lib_location}")
    else()
        # This FATAL_ERROR should ideally not be hit if find_package(Qt6...) succeeded
        message(FATAL_ERROR "Could not find location of linked Qt library: ${lib_target}. Ensure it's correctly installed and find_package found it.")
    endif()
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




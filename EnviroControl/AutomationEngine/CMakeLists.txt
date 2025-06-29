# AutomationEngine

FetchContent_MakeAvailable(googletest)

qt_add_library(AutomationEngine STATIC
    AutomationEngine.h
    automation_engine.cpp
    AutomationWidget.h
    automation_widget.cpp
    AbstractCondition.h
    abstract_condition.cpp
    RuleSet.h
    rule_set.cpp
    RulesProcessor.h
    rules_processor.cpp
    ManualDeviceControlWidget.h
    manual_device_control_widget.cpp
)

target_include_directories(AutomationEngine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # Allows other targets to include this
)

target_link_libraries(AutomationEngine PRIVATE
    Logging
    ErrorDetail
    WeatherStation
    Config
    DeviceController

    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# === For GoogleTests ===
if (WIN32)
# Get the path to Qt's bin directory dynamically
get_target_property(QT_CORE_LOCATION Qt6::Core LOCATION)
get_filename_component(QT_BIN_DIR ${QT_CORE_LOCATION} DIRECTORY)

# Get the path to Qt's plugins directory dynamically
if(DEFINED QT_PLUGINS_DIR)
    set(QT_PLUGINS_ROOT_DIR ${QT_PLUGINS_DIR})
else()
    # Fallback if QT_PLUGINS_DIR is not defined (e.g., older Qt versions or unusual install)
    # This assumes plugins are typically relative to the bin directory.
    get_filename_component(QT_INSTALL_PREFIX ${QT_BIN_DIR} DIRECTORY) # Go up from bin
    set(QT_PLUGINS_ROOT_DIR "${QT_INSTALL_PREFIX}/plugins")
endif()

add_executable(AutomationEngineTests
    tests/test_condition.cpp
    tests/test_rule.cpp
    tests/test_calculate_device_states.cpp
    tests/WeatherDataCreator.h
)

# This ensures it is placed in the common bin directory defined in the main CMakeLists.txt
set_target_properties(AutomationEngineTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>" # Use the main project's output dir
)

target_compile_options(AutomationEngineTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc> # Add /EHsc flag specifically for MSVC compiler
)

target_include_directories(AutomationEngineTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(AutomationEngineTests PRIVATE
    gtest_main
    gtest      
    AutomationEngine       # Link to the AutomationEngine library itself, since its code is being tested
    Logging
    ErrorDetail
    WeatherStation
    Config
    DeviceController

    Qt6::Core  
)

# It copies the 'platforms' subdirectory from Qt's plugins directory to the runtime output directory of test executable
add_custom_command(
    TARGET AutomationEngineTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_PLUGINS_ROOT_DIR}/platforms"
            "$<TARGET_FILE_DIR:AutomationEngineTests>/platforms"
    COMMENT "Copying Qt platform plugins to test executable directory."
)

# Copy core Qt DLLs and other essential runtime DLLs from Qt's bin directory
add_custom_command(
    TARGET AutomationEngineTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_BIN_DIR}/Qt6Cored.dll" "$<TARGET_FILE_DIR:AutomationEngineTests>"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_BIN_DIR}/Qt6Guid.dll" "$<TARGET_FILE_DIR:AutomationEngineTests>"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_BIN_DIR}/Qt6Widgetsd.dll" "$<TARGET_FILE_DIR:AutomationEngineTests>"
    # Add other Qt DLLs if your AutomationEngine or tests explicitly link them AND they are shared
    COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_BIN_DIR}/Qt6Networkd.dll" "$<TARGET_FILE_DIR:AutomationEngineTests>"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_BIN_DIR}/Qt6SerialPortd.dll" "$<TARGET_FILE_DIR:AutomationEngineTests>"
    # MSVC Runtime DLLs: These are typically deployed by the Visual C++ Redistributable.
    # However, if you run into issues, you might need to copy these too,
    # but it's usually better to ensure the redistributable is installed on target systems.
    #COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #        "C:/Windows/System32/vcruntime140d.dll" "$<TARGET_FILE_DIR:AutomationEngineTests>"
    #COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #        "C:/Windows/System32/ucrtbased.dll" "$<TARGET_FILE_DIR:AutomationEngineTests>"
    #COMMENT "Copying essential Qt DLLs to test executable directory."
)

# Discover and add tests for this specific test executable to CTest
include(GoogleTest) # Include again to use gtest_discover_tests in this scope
gtest_discover_tests(AutomationEngineTests
    DISCOVERY_MODE PRE_TEST
    ENVIRONMENT "PATH=$ENV{PATH};${QT_BIN_DIR}"
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:AutomationEngineTests>"
)
endif() # WIN32
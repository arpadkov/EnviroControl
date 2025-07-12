# AutomationEngine

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
    Qt6::Svg
)

# === For GoogleTests ===
if (WIN32)

    add_executable(AutomationEngineTests
        tests/test_condition.cpp
        tests/test_rule.cpp
        tests/test_calculate_device_states.cpp
        tests/WeatherDataCreator.h
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
        AutomationEngine # Link to the AutomationEngine library itself
        Logging
        ErrorDetail
        WeatherStation
        Config
        DeviceController

        Qt6::Core
        Qt6::Gui      # If your tests use any Qt GUI classes
        Qt6::Widgets  # If your tests use any Qt Widgets
        Qt6::Svg      # If your tests directly or indirectly (via AutomationEngine) load SVGs
    )

    # Discover and add tests for this specific test executable to CTest
    include(GoogleTest)
    gtest_discover_tests(AutomationEngineTests
        DISCOVERY_MODE PRE_TEST
        # Adjust ENVIRONMENT and WORKING_DIRECTORY to point to the new TEST_DEPLOY_DIR
        ENVIRONMENT "PATH=$ENV{PATH};${QT_BIN_DIR}" # PATH needs Qt's bin directory
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:AutomationEngineTests>" # This resolves to TEST_DEPLOY_DIR
    )


endif() # WIN32
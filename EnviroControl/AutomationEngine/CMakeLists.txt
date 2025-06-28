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
)
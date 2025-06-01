# Logging

qt_add_library(Logging STATIC
    logging.cpp
    Logging.h
)

target_include_directories(Logging PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Logging PUBLIC Qt6::Core)
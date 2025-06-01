# Config

qt_add_library(Config STATIC
    ConfigParser.h
    config_parser.cpp
)

target_include_directories(Config
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # Allows other targets to include this
)

target_link_libraries(Config PRIVATE
    Qt6::Core
)
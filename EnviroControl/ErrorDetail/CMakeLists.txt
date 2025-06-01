# ErrorDetail

qt_add_library(ErrorDetail STATIC
    ErrorDetail.h
    error_detail.cpp
)

target_include_directories(ErrorDetail
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # Allows other targets to include this
)

target_link_libraries(ErrorDetail PRIVATE
   Qt::Core
)
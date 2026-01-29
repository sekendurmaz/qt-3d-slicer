add_library(io_lib STATIC
    stlreader.h
    stlreader.cpp
)

target_link_libraries(io_lib
    PUBLIC
        Qt6::Core
        Qt6::Gui
)

target_include_directories(io_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

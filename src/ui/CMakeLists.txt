add_library(ui_lib STATIC
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui

)

target_link_libraries(ui_lib
    PUBLIC
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        io_lib
)

target_include_directories(ui_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

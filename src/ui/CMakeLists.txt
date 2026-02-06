add_library(ui_lib STATIC
    mainwindow.h
    mainwindow.cpp


)

target_link_libraries(ui_lib
    PUBLIC
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::OpenGL          # ← OpenGL ekle
        Qt6::OpenGLWidgets   # ← OpenGLWidgets ekle
        io_lib
        rendering_lib
        core_slicing      # ← YENİ EKLE!
)

target_include_directories(ui_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

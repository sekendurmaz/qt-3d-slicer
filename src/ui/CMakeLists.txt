add_library(ui_lib STATIC
    mainwindow.h
    mainwindow.cpp

    widgets/BuildPlatePanel.h      # ← YENİ!
        widgets/BuildPlatePanel.cpp    # ← YENİ!



)

set_target_properties(ui_lib PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
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
        core_slicing
        buildplate

)

target_include_directories(ui_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Rendering library
add_library(rendering_lib STATIC
    Camera.cpp
    MeshRenderer.cpp
)

target_link_libraries(rendering_lib
    PUBLIC
        core_lib
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        # opengl32        # ← EKLE! (Windows OpenGL library)
)

# Platform-specific OpenGL linking
if(WIN32)
    target_link_libraries(rendering_lib PUBLIC opengl32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(rendering_lib PUBLIC GL)
elseif(APPLE)
    target_link_libraries(rendering_lib PUBLIC "-framework OpenGL")
endif()

target_include_directories(rendering_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(rendering_lib PUBLIC cxx_std_17)

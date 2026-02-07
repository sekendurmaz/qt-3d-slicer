# Subdirectories
add_subdirectory(core)
add_subdirectory(io)
add_subdirectory(rendering)
add_subdirectory(ui)

# Main executable
add_executable(${PROJECT_NAME}
    main.cpp
)

# Link libraries (✅ SIRAYA DİKKAT!)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ui_lib
        rendering_lib
        io_lib
        core_slicing
        core_lib
        Qt6::Core
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
)

# Windows için console
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# Subdirectories
add_subdirectory(core)
add_subdirectory(io)
add_subdirectory(ui)

# Main executable
add_executable(${PROJECT_NAME}
    main.cpp
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ui_lib
        io_lib
        core_lib
        Qt6::Core        # ← Core ekle
        Qt6::Widgets
)

# Windows için console'u kapat (opsiyonel)
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

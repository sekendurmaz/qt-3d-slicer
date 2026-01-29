add_subdirectory(ui)
add_subdirectory(io)
add_subdirectory(core)

add_executable(${PROJECT_NAME}
    main.cpp
    core/mesh/mesh.h
    core/mesh/mesh.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        core_lib
        ui_lib
)


if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

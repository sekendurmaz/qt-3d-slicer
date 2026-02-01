add_subdirectory(models)
# add_subdirectory(g_code)
# add_subdirectory(json_config)

add_library(io_lib INTERFACE)

target_link_libraries(io_lib INTERFACE 
    models_lib
    # g_code_lib
    # json_config_lib
)

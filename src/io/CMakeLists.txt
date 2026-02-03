# IO Module
# Input/Output operations: model loading, G-code, config files, etc.

# Model IO (STL, OBJ, 3MF readers)
add_subdirectory(models)

# G-code IO (future)
# add_subdirectory(g_code)

# JSON config IO (future)
# add_subdirectory(json_config)

# Optional: IO interface library (kolaylık için)
add_library(io_lib INTERFACE)

target_link_libraries(io_lib 
    INTERFACE 
        model_io      # models'ten gelen interface library
        # gcode_io    # İleride eklenecek
        # config_io   # İleride eklenecek
)

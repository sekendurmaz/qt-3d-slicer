add_library(stl_io_lib STATIC
    stl_reader.h
    stl_reader.cpp
)

target_include_directories(stl_io_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(stl_io_lib
    PUBLIC
        model_readers_deps
)

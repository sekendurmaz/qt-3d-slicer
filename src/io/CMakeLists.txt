add_subdirectory(stl)

add_library(io_lib INTERFACE)
target_link_libraries(io_lib INTERFACE stl_io_lib)

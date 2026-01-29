add_library(stl_io_lib STATIC
    stl_reader.h
    stl_reader.cpp
)

target_link_libraries(stl_io_lib PUBLIC Qt6::Core Qt6::Gui)
target_include_directories(stl_io_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

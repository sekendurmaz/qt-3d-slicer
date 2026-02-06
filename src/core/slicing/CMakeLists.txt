# Slicing library
add_library(core_slicing STATIC
    Slicer.cpp
)

target_link_libraries(core_slicing
    PUBLIC
        core_lib
)

target_include_directories(core_slicing
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
)


target_compile_features(core_slicing PUBLIC cxx_std_17)

# Slicing library (STATIC - çünkü .cpp var!)
add_library(core_slicing STATIC
    slicer.cpp
    Slicer.h
    Layer.h
    LineSegment.h
    SlicingConstants.h
    ZIndexedMesh.cpp
    ZIndexedMesh.h
)

target_include_directories(core_slicing PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(core_slicing PUBLIC
    core_lib
)

target_compile_features(core_slicing PUBLIC cxx_std_17)

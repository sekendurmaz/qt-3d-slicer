add_library(model_io_stl STATIC
    StlReader.cpp
)

# Include paths
target_include_directories(model_io_stl
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src     
)

# Dependencies
target_link_libraries(model_io_stl
    PUBLIC
        model_io_common  # IModelReader interface
        core_lib         # Mesh, Triangle, Vec3
)

# C++17
target_compile_features(model_io_stl PUBLIC cxx_std_17)
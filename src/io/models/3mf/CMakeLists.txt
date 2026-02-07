# 3MF Reader
add_library(model_io_3mf STATIC
    ThreemfReader.cpp
)

# Include paths
target_include_directories(model_io_3mf
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

# Dependencies
target_link_libraries(model_io_3mf
    PUBLIC
        model_io_common
        core_lib
        tinyxml2
        Qt6::Core
        ZLIB::ZLIB
)

# C++17
target_compile_features(model_io_3mf PUBLIC cxx_std_17)

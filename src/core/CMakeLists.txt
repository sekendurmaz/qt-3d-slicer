# Core library
add_library(core_lib STATIC
    mesh/mesh.cpp
    mesh/MeshValidator.cpp
    mesh/MeshAnalyzer.cpp    # ← YENİ!
)

target_include_directories(core_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(core_lib PUBLIC cxx_std_17)

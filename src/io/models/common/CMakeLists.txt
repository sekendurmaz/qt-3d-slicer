# Model IO - Common (Interface + Factory)
add_library(model_io_common STATIC
    ModelFactory.cpp
)

# Include paths
target_include_directories(model_io_common
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src // src dizini
)

# Dependencies
target_link_libraries(model_io_common
    PUBLIC
        core_lib         # Core'a bağımlı (Mesh kullanıyor)
        model_io_stl     # ← EKLE! STL reader
        model_io_obj     # ← EKLE! OBJ reader
        model_io_3mf     # ← EKLE! 3MF reader
)

# C++17
target_compile_features(model_io_common PUBLIC cxx_std_17)

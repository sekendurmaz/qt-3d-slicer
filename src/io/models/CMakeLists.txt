# Model IO Module
# Desteklenen formatlar: STL, OBJ

# Alt modülleri ekle
add_subdirectory(common)
add_subdirectory(stl)
add_subdirectory(obj)


# Birleştirici interface library (opsiyonel, kolaylık için)
add_library(model_io INTERFACE)

target_link_libraries(model_io
    INTERFACE
        model_io_common
        model_io_stl
        model_io_obj
        
)









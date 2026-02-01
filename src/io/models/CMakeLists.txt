# Ortak bağımlılıklar (tüm model reader'ları core'u görsün)
add_library(model_readers_deps INTERFACE)
target_link_libraries(model_readers_deps
    INTERFACE
        core_lib
)

# Model okuyucuları
# add_subdirectory(3mf)
# add_subdirectory(obj)
add_subdirectory(stl)

# models_lib - tüm model reader'ları toplar
add_library(models_lib INTERFACE)

# Toplayıcı / facade target (uygulama sadece bunu linkler)
target_link_libraries(models_lib 
    INTERFACE 
        stl_io_lib
        # obj_io_lib
        # 3mf_io_lib
)









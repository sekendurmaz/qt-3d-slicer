add_library(core_lib STATIC)

target_sources(core_lib
    PRIVATE
        geometry/vec3.h
        geometry/triangle.h
        geometry/aabb.h
        mesh/mesh.cpp
        mesh/mesh.h
)

target_include_directories(core_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(core_lib PUBLIC cxx_std_17)

# Bilerek target_link_libraries YOK
# Yani core_lib hiçbir kütüphaneye bağlı değil
# core_lib bilerek hiçbir şeye linklenmez:
# - Qt yok
# - io yok
# - ui yok

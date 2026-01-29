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

# Bilerek target_link_libraries YOK
# Yani core_lib hiçbir kütüphaneye bağlı değil

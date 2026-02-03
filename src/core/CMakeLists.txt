# Core library (geometry + mesh)
add_library(core_lib STATIC
    mesh/mesh.cpp)

target_include_directories(core_lib
    PUBLIC  # Diğer kütüphaneler core_lib'i kullanırken bu dizini de dahil etsin
        ${CMAKE_CURRENT_SOURCE_DIR}/..  # → src/
      # ${CMAKE_CURRENT_SOURCE_DIR} # → src/core/
)

#[[ Compiler warnings (opsiyonel ama önerilir)   !!! DENE EN SON
if(MSVC)
    target_compile_options(core_lib PRIVATE /W4)
else()
    target_compile_options(core_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()
]]

target_compile_features(core_lib PUBLIC cxx_std_17)

# CRITICAL: Core has NO dependencies!
# - NO Qt
# - NO io
# - NO ui

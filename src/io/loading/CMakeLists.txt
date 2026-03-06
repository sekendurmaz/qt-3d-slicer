# Loading Strategies Library

add_library(loading_strategies STATIC

    ILoadingStrategy.h
    SyncLoadingStrategy.h
    SyncLoadingStrategy.cpp
    AsyncLoadingStrategy.h
    AsyncLoadingStrategy.cpp
    cache/MeshCache.h
    cache/MeshCache.cpp
    CachedLoadingStrategy.h
    CachedLoadingStrategy.cpp
)

target_include_directories(loading_strategies
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(loading_strategies
    INTERFACE
        core_lib
        model_io
)
target_compile_features(loading_strategies PUBLIC cxx_std_17)

# Thread support for async loading
find_package(Threads REQUIRED)
target_link_libraries(loading_strategies PUBLIC Threads::Threads)

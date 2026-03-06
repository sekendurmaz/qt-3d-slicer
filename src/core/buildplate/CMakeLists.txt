add_library(buildplate
    BuildPlate.h
    RectangularPlate.h
    RectangularPlate.cpp
    CircularPlate.h
    CircularPlate.cpp
)

target_include_directories(buildplate PUBLIC ${CMAKE_SOURCE_DIR}/src)

cmake_minimum_required(VERSION 3.19)

project(MyQtApp
    VERSION 0.1.0
    DESCRIPTION "Qt-based 3D slicer tooling"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt otomatik araçları için
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt paketlerini bul
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui)


#[[ Testing (opsiyonel)
    
    option(BUILD_TESTING "Build tests" ON)
    if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
    endif()
]]

# src alt dizinini ekle
add_subdirectory(src)




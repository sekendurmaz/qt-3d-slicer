cmake_minimum_required(VERSION 3.19)

project(MyQtApp
    VERSION 0.1.0
    DESCRIPTION "Qt-based 3D slicer tooling"
    LANGUAGES CXX C       # ← C ekle!
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt otomatik araçları için
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find ZLIB
find_package(ZLIB REQUIRED)

# Qt paketlerini bul
find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    OpenGL          # ← YENİ!
    OpenGLWidgets   # ← YENİ!
)

add_subdirectory(third_party)

# src alt dizinini ekle
add_subdirectory(src)




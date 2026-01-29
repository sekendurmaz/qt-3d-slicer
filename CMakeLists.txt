cmake_minimum_required(VERSION 3.19)
project(MyQtApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt otomatik araçları için
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt paketlerini bul
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui)

# src alt dizinini ekle
add_subdirectory(src)




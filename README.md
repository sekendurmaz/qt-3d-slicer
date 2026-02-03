
"C:\Qt\6.10.1\mingw_64\bin\designer.exe" src\ui\mainwindow.ui 

MyQtApp/
├── .git/
├── .gitignore
├── .qtcreator/
├── .vscode/
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── build/
│   ├── Desktop_Qt_6_10_1_MinGW_64_bit-Debug/
│   ├── qt-mingw64/
│   └── src/
├── cmake/
└── src/
    ├── CMakeLists.txt
    ├── main.cpp
    ├── core/
    │   ├── CMakeLists.txt
    │   ├── geometry/
    │   └── mesh/
    ├── io/
    │   ├── CMakeLists.txt
    │   ├── readme.md
    │   ├── g_code/
    │   ├── json_config/
    │   └── models/
    │       ├── CMakeLists.txt
    │       ├── common/
    │       │   └── CMakeLists.txt
    │       ├── obj/
    │       ├── stl/
    │       │   ├── CMakeLists.txt
    │       │   ├── stl_reader.cpp
    │       │   └── stl_reader.h
    │       └── 3mf/
    └── ui/
        ├── CMakeLists.txt
        ├── mainwindow.cpp
        ├── mainwindow.h
        └── mainwindow.ui


Geometry = en temel matematiksel yapı taşları
Mesh = bu yapı taşlarından oluşan anlamlı 3B model (Üçgenlerden oluşan model) (STL’den okuduğun binlerce Triangle’ın tek bir kapta toplanmış hali.)
🧠 Neden Triangle Yetmiyor?

      Çünkü gerçek hayatta sen şunu dersin:
      👉 “Bu modelin üçgenleri nerde?”
      👉 “Bu modelin bounding box’ı ne?”
      👉 “Bu modelin adı ne?”
      Bunların hepsi tek bir varlığa ait.
      O varlık: Mesh


Önce en temel katmanlar, sonra onları kullanan katmanlar eklenir. (ROOT/src/CMakeLists.txt)
Bu yüzden:
   core → io → ui   (io core’u bilir, ui hem io’yu hem core’u bilir, ama io ui’yi asla bilmez.)
   doğru sıradır.
   add_subdirectory(core)
   add_subdirectory(io)
   add_subdirectory(ui)

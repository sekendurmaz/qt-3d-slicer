MyQtApp/
├─ CMakeLists.txt
├─ .gitignore
├─ README.md
├─ src/
│  ├─ CMakeLists.txt
│  ├─ main.cpp
│  ├─ ui/
│  │  ├─ CMakeLists.txt
│  │  ├─ mainwindow.h
│  │  ├─ mainwindow.cpp
│  │  └─ mainwindow.ui
│  ├─ io/
│  │  ├─ CMakeLists.txt
│  │  └─ stl/
│  │     ├─ CMakeLists.txt
│  │     ├─ stl_reader.h
│  │     └─ stl_reader.cpp
│  └─ core/
│     ├─ CMakeLists.txt
│     ├─ geometry/...
│     └─ mesh/...
└─ tests/
   └─ CMakeLists.txt


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

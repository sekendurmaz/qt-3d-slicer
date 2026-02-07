# 🖨️ Qt 3D Slicer

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Qt](https://img.shields.io/badge/Qt-6.10-green.svg)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-blue.svg)](https://cmake.org/)

> **⚠️ BETA VERSION** - This is a demonstration/portfolio project showcasing high-performance 3D mesh slicing algorithms and modern C++ techniques. For enterprise-grade solutions with advanced features, please [contact me](#-contact).

A high-performance 3D model slicer built with Qt6 and modern C++17, featuring spatial indexing optimization and multi-format support.

![Screenshot](docs/images/screenshot.png)

---

## ✨ Features

### 🎯 Core Capabilities
- **Multi-Format Support**: STL (Binary/ASCII), OBJ, 3MF
- **High-Performance Slicing**: Spatial indexing (26x speedup)
- **Real-time 3D Visualization**: OpenGL-based mesh rendering
- **Layer Navigation**: Interactive layer-by-layer preview
- **Mesh Analysis**: Volume, surface area, bounds calculation
- **Mesh Processing**: Normal calculation, smoothing, repair

### ⚡ Performance Highlights
| Model | Triangles | Load Time | Slice Time | Speedup |
|-------|-----------|-----------|------------|---------|
| **Bugatti** | 1.47M | 2.3s | 430ms | **26x** 🚀 |
| **Stanford Bunny** | 69K | 85ms | 18ms | **24x** |

**Optimization Techniques:**
- Z-indexed spatial bucketing (O(n*m) → O(n + m*k))
- Memory pre-allocation (reserve capacity)
- Move semantics & perfect forwarding
- Cache-friendly data structures

---

## 📸 Screenshots

<table>
  <tr>
    <td><img src="docs/images/mesh-view.png" alt="Mesh View" width="300"/><br/><i>3D Mesh Visualization</i></td>
    <td><img src="docs/images/layer-view.png" alt="Layer View" width="300"/><br/><i>Layer-by-Layer Preview</i></td>
    <td><img src="docs/images/wireframe.png" alt="Wireframe" width="300"/><br/><i>Wireframe Mode</i></td>
  </tr>
</table>

---

## 🚀 Quick Start

### Prerequisites
- **Qt 6.10+** (Core, Widgets, OpenGL, OpenGLWidgets)
- **CMake 3.16+**
- **C++17 compatible compiler** (GCC 9+, Clang 10+, MSVC 2019+)

### Installation

#### Windows (MinGW)
```bash
git clone https://github.com/sekendurmaz/qt-3d-slicer.git
cd qt-3d-slicer
cmake -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.1/mingw_64"
cmake --build build --config Release
build/src/MyQtApp.exe
```

#### Linux
```bash
git clone https://github.com/sekendurmaz/qt-3d-slicer.git
cd qt-3d-slicer
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/src/MyQtApp
```

#### macOS
```bash
git clone https://github.com/sekendurmaz/qt-3d-slicer.git
cd qt-3d-slicer
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="~/Qt/6.10.1/macos"
cmake --build build
open build/src/MyQtApp.app
```

---

## 🏗️ Architecture

```
qt-3d-slicer/
├── src/
│   ├── core/              # Core algorithms
│   │   ├── geometry/      # Vec3, Triangle, AABB
│   │   ├── mesh/          # Mesh data structure
│   │   └── slicing/       # Slicer, ZIndexedMesh, Layer
│   ├── io/                # File I/O
│   │   └── models/        # STL, OBJ, 3MF readers
│   ├── rendering/         # OpenGL rendering
│   │   ├── MeshRenderer   # 3D mesh + layer rendering
│   │   └── Camera         # Orbit camera controller
│   └── ui/                # Qt UI
│       └── MainWindow     # Main application window
└── third_party/
    └── tinyxml2/          # XML parsing (3MF support)
```

**Design Principles:**
- **Layered Architecture**: `core → io → rendering → ui`
- **Dependency Inversion**: Abstract interfaces, minimal coupling
- **SOLID Principles**: Single responsibility, open/closed
- **Modern C++**: Smart pointers, RAII, move semantics

---

## 🔬 Technical Deep Dive

### Spatial Indexing Algorithm

Traditional slicing: **O(n × m)** - Check every triangle for every layer

```cpp
// Naive approach
for (each layer) {
    for (each triangle) {  // ← Expensive!
        if (triangle intersects layer) {
            extract segment;
        }
    }
}
```

**Our approach:** Z-indexed bucketing **O(n + m × k)**

```cpp
// Build spatial index once
ZIndexedMesh index(mesh, layerHeight);  // O(n)

// Query only relevant triangles per layer
for (each layer) {
    auto tris = index.getTrianglesAtZ(z);  // O(1) lookup
    for (each tri in tris) {  // k << n
        extract segment;
    }
}
```

**Result:** ~26x speedup on large models!

### Memory Optimization

```cpp
// Reserve capacity upfront - avoid reallocations
mesh.reserve(estimatedTriangleCount);

// Move semantics - zero-copy operations
void addTriangle(Triangle&& triangle) {
    triangles.push_back(std::move(triangle));
}

// Emplace back - construct in-place
triangles.emplace_back(v1, v2, v3, normal);
```

---

## 🎯 Roadmap

### ✅ Completed
- [x] Multi-format loading (STL, OBJ, 3MF)
- [x] Spatial indexing optimization
- [x] Layer visualization
- [x] Mesh analysis & repair

### 🚧 In Progress
- [ ] GCode export (G0/G1 moves, temperature control)
- [ ] Infill generation (lines, grid, honeycomb)
- [ ] Support structure generation

### 🔮 Future
- [ ] Multi-threaded slicing (OpenMP)
- [ ] GPU acceleration (Compute shaders)
- [ ] SIMD optimization (SSE/AVX)
- [ ] Advanced infill patterns
- [ ] Variable layer height
- [ ] Adaptive supports

---

## 📊 Performance Benchmarks

**Test System:** Intel i7-10700K, 32GB RAM, Windows 10

| Model | Format | Triangles | Load (Debug) | Load (Release) | Slice (Naive) | Slice (Indexed) | Speedup |
|-------|--------|-----------|--------------|----------------|---------------|-----------------|---------|
| Bugatti | OBJ | 1,471,772 | 2792ms | 2339ms | 11,100ms | 428ms | **26x** |
| Dragon | STL | 871,414 | 1580ms | 1230ms | 6,850ms | 312ms | **22x** |
| Bunny | STL | 69,451 | 102ms | 85ms | 445ms | 18ms | **24x** |

---

## 🤝 Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Setup
```bash
# Clone with submodules
git clone --recursive https://github.com/sekendurmaz/qt-3d-slicer.git

# Build debug version
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 📬 Contact

**Sami Eken Durmaz**

This is a **beta/portfolio version** demonstrating core algorithms and modern C++ techniques. 

**🚀 For production-ready, enterprise-grade solutions with:**
- Advanced path planning algorithms
- Multi-material support
- Cloud integration
- Custom feature development
- Performance consulting

**Please contact me:**

- 📧 Email: [your.email@example.com](mailto:your.email@example.com)
- 💼 LinkedIn: [linkedin.com/in/yourprofile](https://linkedin.com/in/yourprofile)
- 🌐 Portfolio: [yourwebsite.com](https://yourwebsite.com)
- 📱 GitHub: [@sekendurmaz](https://github.com/sekendurmaz)

---

## 🙏 Acknowledgments

- **Qt Framework**: Cross-platform UI framework
- **tinyxml2**: Lightweight XML parsing library
- **Test Models**: Various open-source 3D models from Thingiverse

---

## ⭐ Star History

If you find this project useful, please consider giving it a star! ⭐

[![Star History Chart](https://api.star-history.com/svg?repos=sekendurmaz/qt-3d-slicer&type=Date)](https://star-history.com/#sekendurmaz/qt-3d-slicer&Date)

---

<p align="center">
  <b>Made with ❤️ and modern C++</b><br>
  <sub>© 2025 Sami Eken Durmaz. All rights reserved.</sub>
</p>

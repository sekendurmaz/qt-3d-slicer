# 🖨️ Qt 3D Slicer

[![Build Status](https://github.com/sekendurmaz/qt-3d-slicer/actions/workflows/windows-msvc.yml/badge.svg)](https://github.com/sekendurmaz/qt-3d-slicer/actions/workflows/windows-msvc.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Qt](https://img.shields.io/badge/Qt-6.4+-green.svg)](https://www.qt.io/)
[![Windows](https://img.shields.io/badge/Windows-✓-blue.svg)]()
[![Linux](https://img.shields.io/badge/Linux-✓-orange.svg)]()
[![macOS](https://img.shields.io/badge/macOS-✓-lightgrey.svg)]()

> ** BETA VERSION** - This is a demonstration/portfolio project showcasing high-performance 3D mesh slicing algorithms and modern C++ techniques.

A cross-platform, high-performance 3D model slicer built with Qt6 and modern C++17, featuring spatial indexing optimization (26x speedup) and multi-format support.

![Screenshot](docs/images/screenshot.png)

---

##  What This Is

This is a **beta/portfolio project** that demonstrates:
- ✅ Advanced spatial indexing algorithms (26x performance improvement)
- ✅ Modern C++17 techniques (move semantics, RAII, perfect forwarding)
- ✅ Cross-platform development (Windows, Linux, macOS)
- ✅ Clean architecture (SOLID principles, layered design)
- ✅ Real-time 3D visualization with OpenGL
- ✅ Professional CI/CD pipeline

---

##  Current Features (Beta)

###  Core Capabilities
- **Multi-Format Support**: STL (Binary/ASCII), OBJ
- **High-Performance Slicing**: Spatial indexing with Z-bucketing (26x speedup)
- **Real-time 3D Visualization**: Hardware-accelerated OpenGL rendering
- **Layer Navigation**: Interactive layer-by-layer preview
- **Mesh Analysis**: Volume, surface area, bounds calculation
- **Mesh Processing**: Normal calculation, validation, basic repair

###  Performance Highlights
| Model                 | Triangles     | Load Time | Slice Time | Speedup  |
|-----------------------|---------------|-----------|------------|----------|
| **Model.obj**         | 1.47M         | 2.3s      | 430ms      | **26x**  |
| **Model.stl**         | 871K          | 1.2s      | 312ms      | **22x**  |
|-----------------------|---------------|-----------|------------|----------|

**Optimization Techniques:**
- Z-indexed spatial bucketing (O(n×m) → O(n + m×k))
- Memory pre-allocation and reserve strategy
- Move semantics & perfect forwarding
- Cache-friendly data structures
- SIMD-ready data layout

---

##  Need More Features?

This beta version demonstrates core capabilities. **For enterprise-grade solutions, contact me for:**

###  Additional Format Support
- **3MF** (partial support exists, full implementation available)
- **AMF** (Additive Manufacturing Format)
- **STEP/IGES** (CAD formats)
- **Custom proprietary formats**

###  Performance Enhancements
- **Multi-threaded slicing** (OpenMP/TBB parallelization)
- **GPU acceleration** (CUDA/OpenCL compute shaders)
- **SIMD optimization** (AVX2/AVX-512 vectorization)
- **Memory-mapped I/O** for large files (>1GB)
- **Streaming architecture** for cloud processing

###  Advanced Features
- **GCode generation** with path optimization
- **Infill patterns** (lines, grid, honeycomb, gyroid, adaptive)
- **Support structure generation** (tree supports, organic supports)
- **Variable layer heights** (adaptive slicing)
- **Multi-material support** (MMU/IDEX printers)
- **Cloud integration** (AWS S3, Azure Blob, Google Cloud)
- **REST API** for headless operation
- **Plugin system** for custom algorithms

###  Enterprise Solutions
- **Performance profiling & optimization consulting**
- **Custom algorithm development**
- **Integration with existing pipelines**
- **White-label solutions**
- **Technical training & support**

** Interested? [Contact me]:(#firat13.sd@gmail.com)!**

---

##  Screenshots

<table>
  <tr>
    <td><img src="docs/images/mesh-view.png" alt="Mesh View" width="300"/><br/><i>3D Mesh Visualization</i></td>
    <td><img src="docs/images/layer-view.png" alt="Layer View" width="300"/><br/><i>Layer-by-Layer Preview</i></td>
    <td><img src="docs/images/wireframe.png" alt="Wireframe" width="300"/><br/><i>Wireframe Mode</i></td>
    <td><img src="docs/images/slicer-view.png" alt="slicer-view" width="300"/><br/><i>Wireframe Mode</i></td>
  </tr>
</table>

---

##  Quick Start

###  Pre-built Binaries

Download the latest builds from [GitHub Actions](https://github.com/sekendurmaz/qt-3d-slicer/actions):
- Windows: `MyQtApp-Windows-MSVC-{sha}.zip`
- Linux: `MyQtApp-Linux-{sha}.tar.gz`
- macOS: `MyQtApp-macOS-{sha}.dmg`

### ️ Build from Source

#### Prerequisites
- **Qt 6.4+** (Core, Widgets, OpenGL, OpenGLWidgets)
- **CMake 3.16+**
- **C++17 compatible compiler** (GCC 9+, Clang 10+, MSVC 2019+)

#### Windows (MSVC)
```bash
git clone --recursive https://github.com/sekendurmaz/qt-3d-slicer.git
cd qt-3d-slicer
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.7.2/msvc2019_64"
cmake --build build --config Release
build/src/Release/MyQtApp.exe
```

#### Linux (Ubuntu 22.04+)
```bash
git clone --recursive https://github.com/sekendurmaz/qt-3d-slicer.git
cd qt-3d-slicer

# Install dependencies
sudo apt-get update
sudo apt-get install -y qt6-base-dev libqt6opengl6-dev cmake ninja-build

# Build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/src/MyQtApp
```

#### macOS
```bash
git clone --recursive https://github.com/sekendurmaz/qt-3d-slicer.git
cd qt-3d-slicer

# Install Qt via Homebrew (or download from qt.io)
brew install qt@6 ninja

# Build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6)"
cmake --build build
./build/src/MyQtApp
```

---

## ️ Architecture

```
qt-3d-slicer/
├── src/
│   ├── core/              # Core algorithms (format-agnostic)
│   │   ├── geometry/      # Vec3, Triangle, AABB, math utilities
│   │   ├── mesh/          # Mesh data structure & analysis
│   │   └── slicing/       # Slicer, ZIndexedMesh, Layer
│   ├── io/                # File I/O (pluggable readers)
│   │   └── models/        # STL, OBJ readers (3MF partial)
│   ├── rendering/         # Hardware-accelerated rendering
│   │   ├── MeshRenderer   # 3D mesh + layer visualization
│   │   └── Camera         # Orbit camera controller
│   └── ui/                # Qt UI layer
│       └── MainWindow     # Main application window
└── third_party/
    └── tinyxml2/          # XML parsing (for 3MF support)
```

**Design Principles:**
- **Layered Architecture**: `core → io → rendering → ui` (strict dependency flow)
- **Dependency Inversion**: Abstract interfaces, minimal coupling
- **SOLID Principles**: Single responsibility, open/closed for extension
- **Modern C++17**: Smart pointers, RAII, move semantics, perfect forwarding


##  Roadmap

### ✅ Completed (Beta v0.1)
- [x] Multi-format loading (STL Binary/ASCII, OBJ)
- [x] Spatial indexing optimization (26x speedup)
- [x] Real-time layer visualization
- [x] Mesh analysis & validation
- [x] Cross-platform build (Windows/Linux/macOS)
- [x] CI/CD pipeline (GitHub Actions)

###  In Progress (Contact for Custom Development)
- [ ] **GCode export** (G0/G1 moves, temperature control)
- [ ] **Infill generation** (lines, grid, honeycomb, gyroid)
- [ ] **Support structure generation** (auto-detection, tree supports)
- [ ] **3MF full support** (materials, metadata)

###  Advanced Features (Available on Request)
- [ ] **Multi-threaded slicing** (OpenMP/TBB)
- [ ] **GPU acceleration** (CUDA compute shaders)
- [ ] **SIMD optimization** (AVX2/AVX-512)
- [ ] **Variable layer height** (adaptive slicing)
- [ ] **Cloud API** (REST endpoints, S3 integration)
- [ ] **Plugin system** (custom slicer algorithms)

---

##  Performance Benchmarks

**Test System:** Intel i7-10700K (8C/16T @ 3.8GHz), 32GB DDR4-3200, Windows 10

| Model  Format | Triangles | Load (Debug) | Load (Release) | Slice (Naive) | Slice (Indexed) | Speedup |
|---------------|-----------|--------------|----------------|---------------|-----------------|---------|
|      OBJ      | 1,471,772 | 2792ms       | 2339ms         | 11,100ms      | 428ms           | **26x** |
|      STL      | 871,414   | 1580ms       | 1230ms         | 6,850ms       | 312ms           | **22x** |
|        STL    | 69,451    | 102ms        | 85ms           | 445ms         | 18ms            | **24x** |
|---------------|-----------|--------------|----------------|---------------|-----------------|---------|

*Note: With multi-threading and SIMD, these numbers can be improved 4-8x further. [Contact me](firat13.sd@gmail.com) for custom optimization.*

---

##  Contributing

This is a portfolio/demonstration project. Contributions are welcome for:
-  Bug fixes
-  Documentation improvements
-  Test cases
-  UI/UX enhancements

For major features (GCode export, GPU acceleration, etc.), please contact me.


##  License

This project is licensed under the **MIT License** -
**TL;DR:** Free to use, modify, and distribute. No warranty provided.

---

##  Contact & Custom Development

**Seken Durmaz**  
*High-Performance C++ Developer | Embedded System and 3D Graphics Specialist*

###  Need Custom Solutions?

I offer professional development services for:

####  **3D Processing & Slicing**
- Custom slicer algorithms for specialized printers
- High-throughput batch processing (cloud/server)
- Integration with existing CAM/CAD pipelines
- Performance optimization consulting (10x+ improvements)

####  **Performance Engineering**
- Multi-threaded/GPU acceleration (CUDA, OpenCL, Metal)
- SIMD optimization (AVX2/AVX-512/NEON)
- Memory optimization & cache profiling
- Real-time systems (< 16ms frame time)

####  **Feature Development**
- Additional format support (3MF, AMF, STEP, custom)
- Advanced infill patterns & path planning
- Support structure generation (organic, tree)
- Multi-material & variable layer height
- Cloud API integration (AWS, Azure, GCP)

####  **Consulting & Training**
- Algorithm design & code review
- Performance profiling & bottleneck analysis
- Modern C++ best practices training
- Architecture consulting (microservices, plugins)

###  Get in Touch

**Email:** firat13.sd@gmail.com

**Response Time:** Usually within 24 hours on business days.

---


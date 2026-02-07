#pragma once

#include "core/geometry/vec3.h"

namespace core {
namespace geometry {

// STL'deki bir yüzeyi (facet) temsil eder
struct Triangle
{
    Vec3 normal;        // Yüzey normali
    Vec3 vertex1;       // 1. köşe
    Vec3 vertex2;       // 2. köşe
    Vec3 vertex3;       // 3. köşe

    // ⭐ Constructor ekle
    Triangle() = default;
    Triangle(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& n = Vec3{0,0,0})
        : vertex1(v1), vertex2(v2), vertex3(v3), normal(n)
    {}
};

} // namespace geometry
} // namespace core




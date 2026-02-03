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
};

} // namespace geometry
} // namespace core

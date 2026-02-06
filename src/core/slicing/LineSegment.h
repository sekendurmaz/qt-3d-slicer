#pragma once

#include "core/geometry/vec3.h"
#include <cmath>   // ✅ std::sqrt için şart

namespace core {
namespace slicing {

/**
 * @brief 2D line segment (slicing sonucu)
 */
struct LineSegment
{
    geometry::Vec3 start;  // Başlangıç noktası (z sabit)
    geometry::Vec3 end;    // Bitiş noktası (z sabit)

    LineSegment() = default;

    LineSegment(const geometry::Vec3& s, const geometry::Vec3& e)
        : start(s), end(e)
    {}

    // Length
    float length() const
    {
        float dx = end.x - start.x;
        float dy = end.y - start.y;
        return std::sqrt(dx*dx + dy*dy);
    }
};

} // namespace slicing
} // namespace core

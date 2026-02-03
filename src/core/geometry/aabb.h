#pragma once

namespace core {
namespace geometry {

#include "core/geometry/vec3.h"

struct AABB
{
    Vec3 min;   // En küçük köşe
    Vec3 max;   // En büyük köşe
};

} // namespace geometry
} // namespace core

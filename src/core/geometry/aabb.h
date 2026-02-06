#pragma once
#include "core/geometry/vec3.h"
namespace core {
namespace geometry {



struct AABB
{
    Vec3 min;   // En küçük köşe
    Vec3 max;   // En büyük köşe
};

} // namespace geometry
} // namespace core

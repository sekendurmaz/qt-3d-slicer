#include "mesh.h"

#include <algorithm>
#include <cfloat>

bool Mesh::computeBounds() noexcept
{
    if (triangles.empty())
    {
        bounds = {};
        return false;
    }

    bounds.min = { FLT_MAX,  FLT_MAX,  FLT_MAX };
    bounds.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    auto update = [&](const Vec3& v)
    {
        bounds.min.x = std::min(bounds.min.x, v.x);
        bounds.min.y = std::min(bounds.min.y, v.y);
        bounds.min.z = std::min(bounds.min.z, v.z);

        bounds.max.x = std::max(bounds.max.x, v.x);
        bounds.max.y = std::max(bounds.max.y, v.y);
        bounds.max.z = std::max(bounds.max.z, v.z);
    };

    for (const Triangle& t : triangles)
    {
        update(t.v1);
        update(t.v2);
        update(t.v3);
    }

    return true;
}

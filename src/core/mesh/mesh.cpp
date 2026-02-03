#include "core/mesh/mesh.h" // ← core::mesh::Mesh tanımı
#include <algorithm>        // std::min, std::max
#include <cfloat>           // FLT_MAX  

namespace core {
namespace mesh {                        

bool Mesh::computeBounds() noexcept
{
    if (triangles.empty())
    {
        bounds = {};
        return false;
    }

    bounds.min = { FLT_MAX,  FLT_MAX,  FLT_MAX };
    bounds.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    auto update = [&](const geometry::Vec3& v)  // ← geometry:: eklendi
    {
        bounds.min.x = std::min(bounds.min.x, v.x);
        bounds.min.y = std::min(bounds.min.y, v.y);
        bounds.min.z = std::min(bounds.min.z, v.z);

        bounds.max.x = std::max(bounds.max.x, v.x);
        bounds.max.y = std::max(bounds.max.y, v.y);
        bounds.max.z = std::max(bounds.max.z, v.z);
    };

    for (const geometry::Triangle& t : triangles)  // ← geometry:: eklendi
    {
        update(t.vertex1);
        update(t.vertex2);
        update(t.vertex3);
    }

    return true;
}

} // namespace mesh
} // namespace core
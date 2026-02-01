#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include "geometry/triangle.h"
#include "geometry/aabb.h"

class Mesh
{
public:
    std::string name;
    std::vector<Triangle> triangles;
    AABB bounds{};

    void clear() noexcept
    {
        name.clear();
        triangles.clear();
    }

    size_t triangleCount() const noexcept
    {
        return triangles.size();
    }

    bool computeBounds() noexcept;
};

#endif // MESH_H


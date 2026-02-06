#pragma once

#include <vector>
#include <string>
#include "core/geometry/triangle.h"
#include "core/geometry/aabb.h"

namespace core {
namespace mesh {

class Mesh
{
public:
    std::string name;
    std::vector<geometry::Triangle> triangles; // List of triangles in the mesh
    geometry::AABB bounds{};    // Axis-aligned bounding box (AABB)

    void clear() noexcept       // Resets the mesh to an empty state (Mesh'i temizle)
    {
        name.clear();           // Clear the name
        triangles.clear();   // Clear the triangle list
        bounds = geometry::AABB{}; // Reset bounds to default (empty)
    }

      // Triangle ekle (IO katmanı için KRITIK!)
    void addTriangle(const geometry::Triangle& triangle)
    {
        triangles.push_back(triangle);
    }

    // Overload: 3 vertex + normal ile triangle ekle
    void addTriangle(const geometry::Vec3& v1, 
                    const geometry::Vec3& v2, 
                    const geometry::Vec3& v3,
                    const geometry::Vec3& normal = {0.0f, 0.0f, 0.0f})
    {
        geometry::Triangle tri;
        tri.vertex1 = v1;
        tri.vertex2 = v2;
        tri.vertex3 = v3;
        tri.normal = normal;
        triangles.push_back(tri);
    }

    size_t triangleCount() const noexcept // Triangle sayısı
    {
        return triangles.size();
    }

    // Mesh boş mu?
    bool isEmpty() const noexcept
    {
        return triangles.empty();
    }

    bool computeBounds() noexcept;  // Mesh'in AABB'sini hesapla
};

} // namespace mesh
} // namespace core


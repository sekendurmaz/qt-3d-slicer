#include "MeshRepairer.h"
#include <cmath>
#include <unordered_map>
#include <algorithm>

namespace core {
namespace mesh {

MeshRepairResult MeshRepairer::repair(Mesh& mesh) const
{
    MeshRepairResult result;
    result.originalTriangles = static_cast<int>(mesh.triangles.size());
    result.originalVertices = result.originalTriangles * 3;

    // Step 1: Remove invalid triangles
    auto invalidResult = removeInvalidTriangles(mesh);
    result.invalidTrianglesRemoved = invalidResult.invalidTrianglesRemoved;
    if (invalidResult.invalidTrianglesRemoved > 0)
    {
        result.addAction("Removed " + std::to_string(invalidResult.invalidTrianglesRemoved) +
                         " invalid triangles (NaN/Inf)");
    }

    // Step 2: Remove degenerate triangles
    auto degenerateResult = removeDegenerateTriangles(mesh);
    result.degenerateTrianglesRemoved = degenerateResult.degenerateTrianglesRemoved;
    if (degenerateResult.degenerateTrianglesRemoved > 0)
    {
        result.addAction("Removed " + std::to_string(degenerateResult.degenerateTrianglesRemoved) +
                         " degenerate triangles (area ≈ 0)");
    }

    // Step 3: Remove duplicate vertices
    auto duplicateResult = removeDuplicateVertices(mesh);
    result.verticesMerged = duplicateResult.verticesMerged;
    if (duplicateResult.verticesMerged > 0)
    {
        result.addAction("Merged " + std::to_string(duplicateResult.verticesMerged) +
                         " duplicate vertices");
    }

    // Final stats
    result.finalTriangles = static_cast<int>(mesh.triangles.size());
    result.finalVertices = result.finalTriangles * 3;
    result.trianglesRemoved = result.originalTriangles - result.finalTriangles;

    if (result.actions.empty())
    {
        result.addAction("Mesh is already clean - no repairs needed");
    }

    return result;
}

MeshRepairResult MeshRepairer::removeDuplicateVertices(Mesh& mesh, float tolerance) const
{
    MeshRepairResult result;
    result.originalVertices = static_cast<int>(mesh.triangles.size() * 3);

    if (mesh.triangles.empty())
    {
        result.success = false;
        return result;
    }

    // Build vertex map: position -> unique index
    struct VertexKey {
        int x, y, z;

        bool operator==(const VertexKey& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct VertexKeyHash {
        std::size_t operator()(const VertexKey& k) const {
            return std::hash<int>()(k.x) ^
                   (std::hash<int>()(k.y) << 1) ^
                   (std::hash<int>()(k.z) << 2);
        }
    };

    auto makeKey = [tolerance](const geometry::Vec3& v) -> VertexKey {
        float scale = 1.0f / tolerance;
        return {
            static_cast<int>(std::round(v.x * scale)),
            static_cast<int>(std::round(v.y * scale)),
            static_cast<int>(std::round(v.z * scale))
        };
    };

    std::unordered_map<VertexKey, geometry::Vec3, VertexKeyHash> uniqueVertices;
    int mergedCount = 0;

    // Process each triangle
    for (auto& tri : mesh.triangles)
    {
        // Vertex 1
        auto key1 = makeKey(tri.vertex1);
        auto it1 = uniqueVertices.find(key1);
        if (it1 != uniqueVertices.end())
        {
            tri.vertex1 = it1->second;
            mergedCount++;
        }
        else
        {
            uniqueVertices[key1] = tri.vertex1;
        }

        // Vertex 2
        auto key2 = makeKey(tri.vertex2);
        auto it2 = uniqueVertices.find(key2);
        if (it2 != uniqueVertices.end())
        {
            tri.vertex2 = it2->second;
            mergedCount++;
        }
        else
        {
            uniqueVertices[key2] = tri.vertex2;
        }

        // Vertex 3
        auto key3 = makeKey(tri.vertex3);
        auto it3 = uniqueVertices.find(key3);
        if (it3 != uniqueVertices.end())
        {
            tri.vertex3 = it3->second;
            mergedCount++;
        }
        else
        {
            uniqueVertices[key3] = tri.vertex3;
        }
    }

    result.finalVertices = static_cast<int>(uniqueVertices.size());
    result.verticesMerged = mergedCount;

    return result;
}

MeshRepairResult MeshRepairer::removeDegenerateTriangles(Mesh& mesh, float minArea) const
{
    MeshRepairResult result;
    result.originalTriangles = static_cast<int>(mesh.triangles.size());

    // Remove triangles with area < minArea
    auto it = std::remove_if(mesh.triangles.begin(), mesh.triangles.end(),
                             [this, minArea](const geometry::Triangle& tri) {
                                 return triangleArea(tri) < minArea;
                             });

    size_t removedCount = std::distance(it, mesh.triangles.end());
    mesh.triangles.erase(it, mesh.triangles.end());

    result.finalTriangles = static_cast<int>(mesh.triangles.size());
    result.degenerateTrianglesRemoved = static_cast<int>(removedCount);

    return result;
}

MeshRepairResult MeshRepairer::removeInvalidTriangles(Mesh& mesh) const
{
    MeshRepairResult result;
    result.originalTriangles = static_cast<int>(mesh.triangles.size());

    // Remove triangles with NaN or Inf
    auto it = std::remove_if(mesh.triangles.begin(), mesh.triangles.end(),
                             [this](const geometry::Triangle& tri) {
                                 return hasInvalidCoordinates(tri.vertex1) ||
                                        hasInvalidCoordinates(tri.vertex2) ||
                                        hasInvalidCoordinates(tri.vertex3) ||
                                        hasInvalidCoordinates(tri.normal);
                             });

    size_t removedCount = std::distance(it, mesh.triangles.end());
    mesh.triangles.erase(it, mesh.triangles.end());

    result.finalTriangles = static_cast<int>(mesh.triangles.size());
    result.invalidTrianglesRemoved = static_cast<int>(removedCount);

    return result;
}

float MeshRepairer::triangleArea(const geometry::Triangle& tri) const
{
    // Area = 0.5 * ||(v2-v1) × (v3-v1)||

    float dx1 = tri.vertex2.x - tri.vertex1.x;
    float dy1 = tri.vertex2.y - tri.vertex1.y;
    float dz1 = tri.vertex2.z - tri.vertex1.z;

    float dx2 = tri.vertex3.x - tri.vertex1.x;
    float dy2 = tri.vertex3.y - tri.vertex1.y;
    float dz2 = tri.vertex3.z - tri.vertex1.z;

    // Cross product
    float cx = dy1 * dz2 - dz1 * dy2;
    float cy = dz1 * dx2 - dx1 * dz2;
    float cz = dx1 * dy2 - dy1 * dx2;

    float magnitude = std::sqrt(cx*cx + cy*cy + cz*cz);

    return 0.5f * magnitude;
}

bool MeshRepairer::hasInvalidCoordinates(const geometry::Vec3& v) const
{
    return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z) ||
           std::isinf(v.x) || std::isinf(v.y) || std::isinf(v.z);
}

bool MeshRepairer::verticesEqual(const geometry::Vec3& v1,
                                 const geometry::Vec3& v2,
                                 float tolerance) const
{
    float dx = v1.x - v2.x;
    float dy = v1.y - v2.y;
    float dz = v1.z - v2.z;

    return (dx*dx + dy*dy + dz*dz) < (tolerance * tolerance);
}

} // namespace mesh
} // namespace core

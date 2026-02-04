#include "MeshAnalyzer.h"
#include <cmath>
#include <limits>
#include <algorithm>

namespace core {
namespace mesh {

MeshStatistics MeshAnalyzer::analyze(const Mesh& mesh) const
{
    MeshStatistics stats;

    if (mesh.triangles.empty())
    {
        return stats;  // Boş mesh
    }

    // Temel sayılar
    stats.triangleCount = static_cast<int>(mesh.triangles.size());
    stats.vertexCount = stats.triangleCount * 3;  // Yaklaşık (duplicate'ler var)

    // Bounding box
    stats.bounds = computeBoundingBox(mesh);
    stats.dimensions = geometry::Vec3(
        stats.bounds.max.x - stats.bounds.min.x,
        stats.bounds.max.y - stats.bounds.min.y,
        stats.bounds.max.z - stats.bounds.min.z
        );

    // Surface area
    stats.surfaceArea = computeSurfaceArea(mesh);

    // Volume (signed volume method)
    stats.volume = computeVolume(mesh);

    // Volume hesaplanabildi mi kontrol et
    stats.isWatertight = (std::abs(stats.volume) > 1e-6f);

    // Volume negatifse pozitif yap
    if (stats.volume < 0)
    {
        stats.volume = -stats.volume;
    }

    // Center of mass
    stats.centerOfMass = computeCenterOfMass(mesh);

    return stats;
}

geometry::AABB MeshAnalyzer::computeBoundingBox(const Mesh& mesh) const
{
    geometry::AABB bounds;

    // Başlangıç değerleri
    bounds.min = geometry::Vec3(
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
        );
    bounds.max = geometry::Vec3(
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest()
        );

    // Her vertex'i kontrol et
    for (const auto& tri : mesh.triangles)
    {
        // Vertex 1
        bounds.min.x = std::min(bounds.min.x, tri.vertex1.x);
        bounds.min.y = std::min(bounds.min.y, tri.vertex1.y);
        bounds.min.z = std::min(bounds.min.z, tri.vertex1.z);

        bounds.max.x = std::max(bounds.max.x, tri.vertex1.x);
        bounds.max.y = std::max(bounds.max.y, tri.vertex1.y);
        bounds.max.z = std::max(bounds.max.z, tri.vertex1.z);

        // Vertex 2
        bounds.min.x = std::min(bounds.min.x, tri.vertex2.x);
        bounds.min.y = std::min(bounds.min.y, tri.vertex2.y);
        bounds.min.z = std::min(bounds.min.z, tri.vertex2.z);

        bounds.max.x = std::max(bounds.max.x, tri.vertex2.x);
        bounds.max.y = std::max(bounds.max.y, tri.vertex2.y);
        bounds.max.z = std::max(bounds.max.z, tri.vertex2.z);

        // Vertex 3
        bounds.min.x = std::min(bounds.min.x, tri.vertex3.x);
        bounds.min.y = std::min(bounds.min.y, tri.vertex3.y);
        bounds.min.z = std::min(bounds.min.z, tri.vertex3.z);

        bounds.max.x = std::max(bounds.max.x, tri.vertex3.x);
        bounds.max.y = std::max(bounds.max.y, tri.vertex3.y);
        bounds.max.z = std::max(bounds.max.z, tri.vertex3.z);
    }

    return bounds;
}

float MeshAnalyzer::computeSurfaceArea(const Mesh& mesh) const
{
    float totalArea = 0.0f;

    for (const auto& tri : mesh.triangles)
    {
        totalArea += triangleArea(tri);
    }

    return totalArea;
}

float MeshAnalyzer::computeVolume(const Mesh& mesh) const
{
    // Signed volume method (divergence theorem)
    // V = (1/6) * Σ (p · (q × r))

    float totalVolume = 0.0f;

    for (const auto& tri : mesh.triangles)
    {
        totalVolume += signedVolumeOfTriangle(tri);
    }

    return totalVolume;
}

geometry::Vec3 MeshAnalyzer::computeCenterOfMass(const Mesh& mesh) const
{
    geometry::Vec3 center(0, 0, 0);
    int totalVertices = 0;

    for (const auto& tri : mesh.triangles)
    {
        center.x += tri.vertex1.x + tri.vertex2.x + tri.vertex3.x;
        center.y += tri.vertex1.y + tri.vertex2.y + tri.vertex3.y;
        center.z += tri.vertex1.z + tri.vertex2.z + tri.vertex3.z;
        totalVertices += 3;
    }

    if (totalVertices > 0)
    {
        center.x /= totalVertices;
        center.y /= totalVertices;
        center.z /= totalVertices;
    }

    return center;
}

float MeshAnalyzer::triangleArea(const geometry::Triangle& tri) const
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

    // Magnitude
    float magnitude = std::sqrt(cx*cx + cy*cy + cz*cz);

    return 0.5f * magnitude;
}

float MeshAnalyzer::signedVolumeOfTriangle(const geometry::Triangle& tri) const
{
    // Signed volume of tetrahedron formed by triangle and origin
    // V = (1/6) * (p · (q × r))

    const auto& p = tri.vertex1;
    const auto& q = tri.vertex2;
    const auto& r = tri.vertex3;

    // q × r
    float cx = q.y * r.z - q.z * r.y;
    float cy = q.z * r.x - q.x * r.z;
    float cz = q.x * r.y - q.y * r.x;

    // p · (q × r)
    float dot = p.x * cx + p.y * cy + p.z * cz;

    return dot / 6.0f;
}

} // namespace mesh
} // namespace core

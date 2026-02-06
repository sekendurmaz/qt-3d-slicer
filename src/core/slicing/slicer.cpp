#include "Slicer.h"
#include <cmath>
#include <algorithm>

namespace core {
namespace slicing {

SlicingResult Slicer::slice(const mesh::Mesh& mesh, const SlicingSettings& settings)
{
    SlicingResult result;

    if (mesh.triangles.empty())
    {
        return result;
    }

    // Z bounds
    float minZ = settings.minZ;
    float maxZ = settings.maxZ;

    if (maxZ <= minZ)
    {
        // Auto-detect
        getBoundsZ(mesh, minZ, maxZ);
    }

    result.layerHeight = settings.layerHeight;
    result.totalHeight = maxZ - minZ;

    // Layer sayısı
    int layerCount = static_cast<int>(std::ceil((maxZ - minZ) / settings.layerHeight));

    if (layerCount <= 0)
    {
        return result;
    }

    result.layers.reserve(layerCount);

    // Her Z seviyesinde slice
    for (int i = 0; i < layerCount; ++i)
    {
        float z = minZ + i * settings.layerHeight;

        Layer layer = sliceAtZ(mesh, z);

        if (!layer.isEmpty())
        {
            result.layers.push_back(layer);
            result.totalSegments += static_cast<int>(layer.segmentCount());
        }
    }

    result.success = !result.layers.empty();

    return result;
}

Layer Slicer::sliceAtZ(const mesh::Mesh& mesh, float z)
{
    Layer layer(z);

    // Her triangle'ı kontrol et
    for (const auto& tri : mesh.triangles)
    {
        LineSegment segment;

        if (intersectTriangleWithPlane(tri, z, segment))
        {
            layer.addSegment(segment);
        }
    }

    return layer;
}

bool Slicer::intersectTriangleWithPlane(const geometry::Triangle& tri,
                                        float z,
                                        LineSegment& outSegment)
{
    // Triangle'ın 3 vertex'i
    const auto& v1 = tri.vertex1;
    const auto& v2 = tri.vertex2;
    const auto& v3 = tri.vertex3;

    // Her vertex'in Z plane'e göre konumu
    bool above1 = v1.z > z;
    bool above2 = v2.z > z;
    bool above3 = v3.z > z;

    // Kaç vertex üstte?
    int aboveCount = (above1 ? 1 : 0) + (above2 ? 1 : 0) + (above3 ? 1 : 0);

    // Kesişim yok (hepsi üstte veya hepsi altta)
    if (aboveCount == 0 || aboveCount == 3)
    {
        return false;
    }

    // Kesişim noktalarını bul
    std::vector<geometry::Vec3> intersections;

    // Edge 1-2
    if (above1 != above2)
    {
        float t = (z - v1.z) / (v2.z - v1.z);
        geometry::Vec3 p(
            v1.x + t * (v2.x - v1.x),
            v1.y + t * (v2.y - v1.y),
            z
        );
        intersections.push_back(p);
    }

    // Edge 2-3
    if (above2 != above3)
    {
        float t = (z - v2.z) / (v3.z - v2.z);
        geometry::Vec3 p(
            v2.x + t * (v3.x - v2.x),
            v2.y + t * (v3.y - v2.y),
            z
        );
        intersections.push_back(p);
    }

    // Edge 3-1
    if (above3 != above1)
    {
        float t = (z - v3.z) / (v1.z - v3.z);
        geometry::Vec3 p(
            v3.x + t * (v1.x - v3.x),
            v3.y + t * (v1.y - v3.y),
            z
        );
        intersections.push_back(p);
    }

    // Tam 2 kesişim olmalı
    if (intersections.size() == 2)
    {
        outSegment = LineSegment(intersections[0], intersections[1]);
        return true;
    }

    return false;
}

void Slicer::getBoundsZ(const mesh::Mesh& mesh, float& minZ, float& maxZ)
{
    minZ = std::numeric_limits<float>::max();
    maxZ = std::numeric_limits<float>::lowest();

    for (const auto& tri : mesh.triangles)
    {
        minZ = std::min({minZ, tri.vertex1.z, tri.vertex2.z, tri.vertex3.z});
        maxZ = std::max({maxZ, tri.vertex1.z, tri.vertex2.z, tri.vertex3.z});
    }
}

} // namespace slicing
} // namespace core

#include "Slicer.h"
#include "ZIndexedMesh.h"
#include "SlicingConstants.h"
#include <cmath>
#include <algorithm>
#include <array>


namespace core {
namespace slicing {

SlicingResult Slicer::slice(const mesh::Mesh& mesh, const SlicingSettings& settings)
{
    SlicingResult result;

    // Validation
    if (mesh.triangles.empty())
    {
        result.error = SlicingError::EmptyMesh;
        result.errorMessage = "Mesh contains no triangles";
        return result;
    }

    if (settings.layerHeight < MIN_LAYER_HEIGHT ||
        settings.layerHeight > MAX_LAYER_HEIGHT)
    {
        result.error = SlicingError::InvalidLayerHeight;
        result.errorMessage = "Layer height must be between " +
                              std::to_string(MIN_LAYER_HEIGHT) + " and " +
                              std::to_string(MAX_LAYER_HEIGHT) + " mm";
        return result;
    }

    float minZ = settings.minZ;
    float maxZ = settings.maxZ;

    if (maxZ <= minZ + EPSILON)
    {
        getBoundsZ(mesh, minZ, maxZ);

        if (maxZ <= minZ + EPSILON)
        {
            result.error = SlicingError::InvalidBounds;
            result.errorMessage = "Mesh has zero or negative height";
            return result;
        }
    }

    result.layerHeight = settings.layerHeight;
    result.totalHeight = maxZ - minZ;

    int layerCount = static_cast<int>(std::ceil((maxZ - minZ) / settings.layerHeight));

    if (layerCount <= 0)
    {
        result.error = SlicingError::InvalidBounds;
        result.errorMessage = "Calculated layer count is zero or negative";
        return result;
    }

    if (layerCount > MAX_LAYER_COUNT)
    {
        result.error = SlicingError::TooManyLayers;
        result.errorMessage = "Layer count exceeds maximum";
        return result;
    }

    result.layers.reserve(layerCount);

    // Spatial indexing (SESLİ DEĞİL - SADECE ÇALIŞIR!)
    if (settings.useSpatialIndex)
    {
        ZIndexedMesh indexedMesh(mesh, settings.layerHeight);

        for (int i = 0; i < layerCount; ++i)
        {
            float z = minZ + i * settings.layerHeight;
            Layer layer = sliceAtZ(indexedMesh, z);

            if (!layer.isEmpty())
            {
                result.layers.push_back(layer);
                result.totalSegments += static_cast<int>(layer.segmentCount());
            }
        }
    }
    else
    {
        // Naive slicing
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
    }

    if (result.layers.empty())
    {
        result.error = SlicingError::NoIntersections;
        result.errorMessage = "No intersections found";
    }
    else
    {
        result.error = SlicingError::Success;
    }

    return result;
}

Layer Slicer::sliceAtZ(const ZIndexedMesh& indexedMesh, float z)
{
    Layer layer(z);

    auto triangles = indexedMesh.getTrianglesAtZ(z);

    for (const auto* tri : triangles)
    {
        LineSegment segment;

        if (intersectTriangleWithPlane(*tri, z, segment))
        {
            layer.addSegment(segment);
        }
    }

    return layer;
}

Layer Slicer::sliceAtZ(const mesh::Mesh& mesh, float z)
{
    Layer layer(z);

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
    const auto& v1 = tri.vertex1;
    const auto& v2 = tri.vertex2;
    const auto& v3 = tri.vertex3;

    VertexPosition pos1 = classifyVertex(v1.z, z);
    VertexPosition pos2 = classifyVertex(v2.z, z);
    VertexPosition pos3 = classifyVertex(v3.z, z);

    int aboveCount = (pos1 == VertexPosition::Above ? 1 : 0) +
                     (pos2 == VertexPosition::Above ? 1 : 0) +
                     (pos3 == VertexPosition::Above ? 1 : 0);

    if (aboveCount == 0 || aboveCount == 3)
    {
        return false;
    }

    std::array<geometry::Vec3, 2> intersections;
    size_t intersectionCount = 0;

    if (pos1 != pos2)
    {
        geometry::Vec3 point;
        if (interpolateEdge(v1, v2, z, point))
        {
            intersections[intersectionCount++] = point;
        }
    }

    if (pos2 != pos3 && intersectionCount < 2)
    {
        geometry::Vec3 point;
        if (interpolateEdge(v2, v3, z, point))
        {
            intersections[intersectionCount++] = point;
        }
    }

    if (pos3 != pos1 && intersectionCount < 2)
    {
        geometry::Vec3 point;
        if (interpolateEdge(v3, v1, z, point))
        {
            intersections[intersectionCount++] = point;
        }
    }

    if (intersectionCount == 2)
    {
        outSegment = LineSegment(intersections[0], intersections[1]);
        return true;
    }

    return false;
}

VertexPosition Slicer::classifyVertex(float vz, float planeZ) const
{
    if (vz > planeZ + EPSILON)
    {
        return VertexPosition::Above;
    }
    else if (vz < planeZ - EPSILON)
    {
        return VertexPosition::Below;
    }
    else
    {
        return VertexPosition::On;
    }
}

bool Slicer::interpolateEdge(const geometry::Vec3& p1,
                             const geometry::Vec3& p2,
                             float z,
                             geometry::Vec3& outPoint) const
{
    float dz = p2.z - p1.z;

    if (std::abs(dz) < EPSILON)
    {
        return false;
    }

    float t = (z - p1.z) / dz;

    if (t < -EPSILON || t > 1.0f + EPSILON)
    {
        return false;
    }

    t = std::clamp(t, 0.0f, 1.0f);

    outPoint = geometry::Vec3(
        p1.x + t * (p2.x - p1.x),
        p1.y + t * (p2.y - p1.y),
        z
        );

    return true;
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

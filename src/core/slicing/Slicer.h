#pragma once

#include "Layer.h"
#include "LineSegment.h"
#include "SlicingConstants.h"
#include "core/mesh/mesh.h"
#include <vector>
#include <string>

namespace core {
namespace slicing {

// ⭐ Forward declaration
class ZIndexedMesh;

enum class SlicingError
{
    Success = 0,
    EmptyMesh,
    InvalidLayerHeight,
    InvalidBounds,
    TooManyLayers,
    NoIntersections
};

enum class VertexPosition
{
    Above,
    On,
    Below
};

struct SlicingSettings
{
    float layerHeight = 0.2f;
    float minZ = 0.0f;
    float maxZ = 0.0f;
    bool useSpatialIndex = true;
};

struct SlicingResult
{
    std::vector<Layer> layers;

    int totalSegments = 0;
    float totalHeight = 0.0f;
    float layerHeight = 0.0f;

    SlicingError error = SlicingError::Success;
    std::string errorMessage;

    bool success() const { return error == SlicingError::Success; }
};

class Slicer
{
public:
    Slicer() = default;

    SlicingResult slice(const mesh::Mesh& mesh, const SlicingSettings& settings);

private:
    // ⭐ İKİ AYRI OVERLOAD (açıkça belirt!)
    Layer sliceAtZ(const mesh::Mesh& mesh, float z);
    Layer sliceAtZ(const ZIndexedMesh& indexedMesh, float z);  // ← Forward declaration yeterli

    bool intersectTriangleWithPlane(const geometry::Triangle& tri,
                                    float z,
                                    LineSegment& outSegment);

    void getBoundsZ(const mesh::Mesh& mesh, float& minZ, float& maxZ);

    VertexPosition classifyVertex(float vz, float planeZ) const;

    bool interpolateEdge(const geometry::Vec3& p1,
                         const geometry::Vec3& p2,
                         float z,
                         geometry::Vec3& outPoint) const;
};

} // namespace slicing
} // namespace core

#include "NormalProcessor.h"
#include <cmath>
#include <unordered_map>
#include <vector>

namespace core {
namespace mesh {

NormalProcessingResult NormalProcessor::recalculateNormals(Mesh& mesh) const
{
    NormalProcessingResult result;

    if (mesh.triangles.empty())
    {
        result.success = false;
        return result;
    }

    // Recalculate normal for each triangle
    for (auto& tri : mesh.triangles)
    {
        tri.normal = computeNormal(tri);
        result.normalsRecalculated++;
    }

    return result;
}

NormalProcessingResult NormalProcessor::smoothNormals(Mesh& mesh, float angleThreshold) const
{
    NormalProcessingResult result;

    if (mesh.triangles.empty())
    {
        result.success = false;
        return result;
    }

    // Build vertex → triangles map
    // Key: vertex position (rounded), Value: list of triangle indices
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

    std::unordered_map<VertexKey, std::vector<size_t>, VertexKeyHash> vertexMap;

    auto makeKey = [](const geometry::Vec3& v) -> VertexKey {
        const float scale = 10000.0f;
        return {
            static_cast<int>(v.x * scale),
            static_cast<int>(v.y * scale),
            static_cast<int>(v.z * scale)
        };
    };

    // Populate map
    for (size_t i = 0; i < mesh.triangles.size(); ++i)
    {
        const auto& tri = mesh.triangles[i];
        vertexMap[makeKey(tri.vertex1)].push_back(i);
        vertexMap[makeKey(tri.vertex2)].push_back(i);
        vertexMap[makeKey(tri.vertex3)].push_back(i);
    }

    // Smooth each triangle's normal
    std::vector<geometry::Vec3> newNormals(mesh.triangles.size());

    for (size_t i = 0; i < mesh.triangles.size(); ++i)
    {
        const auto& tri = mesh.triangles[i];
        geometry::Vec3 avgNormal = tri.normal;
        int count = 1;

        // Find neighbors (triangles sharing vertices)
        auto addNeighbors = [&](const geometry::Vec3& vertex) {
            auto it = vertexMap.find(makeKey(vertex));
            if (it != vertexMap.end())
            {
                for (size_t neighborIdx : it->second)
                {
                    if (neighborIdx != i)
                    {
                        const auto& neighborNormal = mesh.triangles[neighborIdx].normal;

                        // Only average if angle is small enough
                        float dotProd = dot(tri.normal, neighborNormal);
                        float angleRad = std::acos(std::min(std::max(dotProd, -1.0f), 1.0f));
                        float angleDeg = angleRad * 180.0f / 3.14159f;

                        if (angleDeg < angleThreshold)
                        {
                            avgNormal.x += neighborNormal.x;
                            avgNormal.y += neighborNormal.y;
                            avgNormal.z += neighborNormal.z;
                            count++;
                        }
                    }
                }
            }
        };

        addNeighbors(tri.vertex1);
        addNeighbors(tri.vertex2);
        addNeighbors(tri.vertex3);

        // Average
        if (count > 1)
        {
            avgNormal.x /= count;
            avgNormal.y /= count;
            avgNormal.z /= count;
            newNormals[i] = normalize(avgNormal);
            result.normalsSmoothed++;
        }
        else
        {
            newNormals[i] = tri.normal;
        }
    }

    // Apply new normals
    for (size_t i = 0; i < mesh.triangles.size(); ++i)
    {
        mesh.triangles[i].normal = newNormals[i];
    }

    return result;
}

NormalProcessingResult NormalProcessor::flipNormals(Mesh& mesh) const
{
    NormalProcessingResult result;

    for (auto& tri : mesh.triangles)
    {
        tri.normal.x = -tri.normal.x;
        tri.normal.y = -tri.normal.y;
        tri.normal.z = -tri.normal.z;
        result.normalsFlipped++;
    }

    return result;
}

geometry::Vec3 NormalProcessor::computeNormal(const geometry::Triangle& tri) const
{
    // Edge vectors
    float dx1 = tri.vertex2.x - tri.vertex1.x;
    float dy1 = tri.vertex2.y - tri.vertex1.y;
    float dz1 = tri.vertex2.z - tri.vertex1.z;

    float dx2 = tri.vertex3.x - tri.vertex1.x;
    float dy2 = tri.vertex3.y - tri.vertex1.y;
    float dz2 = tri.vertex3.z - tri.vertex1.z;

    // Cross product
    float nx = dy1 * dz2 - dz1 * dy2;
    float ny = dz1 * dx2 - dx1 * dz2;
    float nz = dx1 * dy2 - dy1 * dx2;

    return normalize(geometry::Vec3(nx, ny, nz));
}

geometry::Vec3 NormalProcessor::normalize(const geometry::Vec3& v) const
{
    float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    if (length < 1e-8f)
    {
        return geometry::Vec3(0, 0, 1); // Default up
    }

    return geometry::Vec3(
        v.x / length,
        v.y / length,
        v.z / length
        );
}

float NormalProcessor::dot(const geometry::Vec3& a, const geometry::Vec3& b) const
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

bool NormalProcessor::verticesEqual(const geometry::Vec3& v1,
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

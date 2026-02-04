#include "MeshValidator.h"
#include <cmath>
#include <unordered_set>
#include <sstream>

namespace core {
namespace mesh {

ValidationResult MeshValidator::validate(const Mesh& mesh) const
{
    ValidationResult result;

    // Boş mesh kontrolü
    if (mesh.triangles.empty())
    {
        result.addWarning("Mesh is empty (no triangles)");
        return result;
    }

    // Her triangle'ı kontrol et
    for (size_t i = 0; i < mesh.triangles.size(); ++i)
    {
        const auto& tri = mesh.triangles[i];

        // 1. Invalid koordinat kontrolü (NaN, Inf)
        if (hasInvalidCoordinates(tri.vertex1) ||
            hasInvalidCoordinates(tri.vertex2) ||
            hasInvalidCoordinates(tri.vertex3))
        {
            result.invalidVertices++;
            std::ostringstream oss;
            oss << "Triangle " << i << " has invalid coordinates (NaN/Inf)";
            result.addError(oss.str());
            continue;
        }

        // 2. Degenerate triangle kontrolü (alan ≈ 0)
        if (isDegenerate(tri))
        {
            result.degenerateTriangles++;
            std::ostringstream oss;
            oss << "Triangle " << i << " is degenerate (area ≈ 0)";
            result.addWarning(oss.str());
        }
    }

    // 3. Duplicate vertex kontrolü (basit versiyon)
    // Not: Tam duplicate detection için spatial hash gerekir
    // Şimdilik basit bir kontrol yapalım
    std::unordered_set<std::string> vertexSet;
    int totalVertices = mesh.triangles.size() * 3;

    for (const auto& tri : mesh.triangles)
    {
        auto hashVertex = [](const geometry::Vec3& v) -> std::string {
            std::ostringstream oss;
            oss << std::fixed << v.x << "," << v.y << "," << v.z;
            return oss.str();
        };

        vertexSet.insert(hashVertex(tri.vertex1));
        vertexSet.insert(hashVertex(tri.vertex2));
        vertexSet.insert(hashVertex(tri.vertex3));
    }

    result.duplicateVertices = totalVertices - static_cast<int>(vertexSet.size());

    if (result.duplicateVertices > totalVertices / 10) // %10'dan fazla duplicate
    {
        std::ostringstream oss;
        oss << "Many duplicate vertices detected: " << result.duplicateVertices;
        result.addWarning(oss.str());
    }

    // Özet
    if (result.isValid)
    {
        if (result.warnings.empty())
        {
            result.addWarning("Mesh validation passed with no issues");
        }
    }

    return result;
}

bool MeshValidator::isDegenerate(const geometry::Triangle& tri) const
{
    float area = triangleArea(tri);
    return area < minTriangleArea_;
}

bool MeshValidator::hasInvalidCoordinates(const geometry::Vec3& v) const
{
    return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z) ||
           std::isinf(v.x) || std::isinf(v.y) || std::isinf(v.z);
}

float MeshValidator::triangleArea(const geometry::Triangle& tri) const
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

bool MeshValidator::verticesEqual(const geometry::Vec3& v1,
                                  const geometry::Vec3& v2) const
{
    float dx = v1.x - v2.x;
    float dy = v1.y - v2.y;
    float dz = v1.z - v2.z;

    float distSq = dx*dx + dy*dy + dz*dz;

    return distSq < (vertexTolerance_ * vertexTolerance_);
}

} // namespace mesh
} // namespace core

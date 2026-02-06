#pragma once

#include "mesh.h"
#include <string>
#include <vector>

namespace core {
namespace mesh {

/**
 * @brief Mesh repair report
 */
struct MeshRepairResult
{
    bool success = true;

    // Statistics
    int originalTriangles = 0;
    int finalTriangles = 0;
    int trianglesRemoved = 0;

    int originalVertices = 0;
    int finalVertices = 0;
    int verticesMerged = 0;

    int degenerateTrianglesRemoved = 0;
    int invalidTrianglesRemoved = 0;

    // Summary
    std::vector<std::string> actions;

    void addAction(const std::string& action) {
        actions.push_back(action);
    }
};

/**
 * @brief Mesh repair operations
 */
class MeshRepairer
{
public:
    MeshRepairer() = default;

    /**
     * @brief Full repair (all operations)
     */
    MeshRepairResult repair(Mesh& mesh) const;

    /**
     * @brief Remove duplicate vertices
     * Merges vertices that are within tolerance distance
     * @param tolerance Distance threshold (default: 1e-5)
     */
    MeshRepairResult removeDuplicateVertices(Mesh& mesh, float tolerance = 1e-5f) const;

    /**
     * @brief Remove degenerate triangles
     * Removes triangles with area < threshold
     * @param minArea Minimum area threshold (default: 1e-6)
     */
    MeshRepairResult removeDegenerateTriangles(Mesh& mesh, float minArea = 1e-6f) const;

    /**
     * @brief Remove triangles with invalid coordinates
     * Removes triangles containing NaN or Inf values
     */
    MeshRepairResult removeInvalidTriangles(Mesh& mesh) const;

private:
    // Helpers
    float triangleArea(const geometry::Triangle& tri) const;
    bool hasInvalidCoordinates(const geometry::Vec3& v) const;
    bool verticesEqual(const geometry::Vec3& v1, const geometry::Vec3& v2,
                       float tolerance) const;
};

} // namespace mesh
} // namespace core

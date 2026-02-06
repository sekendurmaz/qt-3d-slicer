#pragma once

#include "mesh.h"

namespace core {
namespace mesh {

/**
 * @brief Normal processing results
 */
struct NormalProcessingResult
{
    int normalsRecalculated = 0;
    int normalsSmoothed = 0;
    int normalsFlipped = 0;
    bool success = true;
};

/**
 * @brief Normal vector processing
 */
class NormalProcessor
{
public:
    NormalProcessor() = default;

    /**
     * @brief Recalculate all triangle normals
     * Uses cross product: (v2-v1) × (v3-v1)
     */
    NormalProcessingResult recalculateNormals(Mesh& mesh) const;

    /**
     * @brief Smooth normals by averaging neighbors
     * Creates smoother shading (but changes geometry slightly)
     * @param angleThreshold Only smooth if angle < threshold (degrees)
     */
    NormalProcessingResult smoothNormals(Mesh& mesh, float angleThreshold = 30.0f) const;

    /**
     * @brief Flip all normals (reverse direction)
     * Useful for inside-out meshes
     */
    NormalProcessingResult flipNormals(Mesh& mesh) const;

private:
    // Helper: compute normal for a triangle
    geometry::Vec3 computeNormal(const geometry::Triangle& tri) const;

    // Helper: normalize vector
    geometry::Vec3 normalize(const geometry::Vec3& v) const;

    // Helper: dot product
    float dot(const geometry::Vec3& a, const geometry::Vec3& b) const;

    // Helper: check if two vertices are equal (within tolerance)
    bool verticesEqual(const geometry::Vec3& v1, const geometry::Vec3& v2,
                       float tolerance = 1e-5f) const;
};

} // namespace mesh
} // namespace core

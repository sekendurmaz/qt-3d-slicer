#pragma once

#include "mesh.h"
#include "core/geometry/vec3.h"
#include "core/geometry/aabb.h"

namespace core {
namespace mesh {

/**
 * @brief Mesh istatistikleri
 */
struct MeshStatistics
{
    // Temel bilgiler
    int triangleCount = 0;
    int vertexCount = 0;              // Unique vertices (yaklaşık)

    // Bounding box
    geometry::AABB bounds;
    geometry::Vec3 dimensions;        // width, height, depth

    // Geometrik özellikler
    float surfaceArea = 0.0f;         // mm² (veya birim²)
    float volume = 0.0f;              // mm³ (veya birim³)

    // Merkez
    geometry::Vec3 centerOfMass;      // Geometrik merkez

    // Yardımcı
    bool isWatertight = false;        // Hacim hesaplanabildi mi?
};

/**
 * @brief Mesh analiz sınıfı
 */
class MeshAnalyzer
{
public:
    MeshAnalyzer() = default;

    /**
     * @brief Mesh'i analiz eder
     * @param mesh Analiz edilecek mesh
     * @return MeshStatistics
     */
    MeshStatistics analyze(const Mesh& mesh) const;

private:
    // Yardımcı fonksiyonlar
    geometry::AABB computeBoundingBox(const Mesh& mesh) const;
    float computeSurfaceArea(const Mesh& mesh) const;
    float computeVolume(const Mesh& mesh) const;
    geometry::Vec3 computeCenterOfMass(const Mesh& mesh) const;

    // Tek triangle için
    float triangleArea(const geometry::Triangle& tri) const;
    float signedVolumeOfTriangle(const geometry::Triangle& tri) const;
};

} // namespace mesh
} // namespace core

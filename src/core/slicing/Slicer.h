#pragma once

#include "Layer.h"
#include "core/mesh/mesh.h"
#include <vector>

namespace core {
namespace slicing {

/**
 * @brief Slicing parametreleri
 */
struct SlicingSettings
{
    float layerHeight = 0.2f;      // Katman yüksekliği (mm)
    float minZ = 0.0f;             // Başlangıç Z
    float maxZ = 0.0f;             // Bitiş Z (0 = auto-detect)

    // Gelecek özellikler için:
    // float infillDensity = 0.2f;
    // int wallCount = 2;
};

/**
 * @brief Slicing sonuçları
 */
struct SlicingResult
{
    std::vector<Layer> layers;

    int totalSegments = 0;
    float totalHeight = 0.0f;
    float layerHeight = 0.0f;

    bool success = false;
};

/**
 * @brief Mesh slicer - mesh'i katmanlara böler
 */
class Slicer
{
public:
    Slicer() = default;

    /**
     * @brief Mesh'i slice et
     * @param mesh Slice edilecek mesh
     * @param settings Slicing parametreleri
     * @return Slicing sonuçları
     */
    SlicingResult slice(const mesh::Mesh& mesh, const SlicingSettings& settings);

private:
    /**
     * @brief Tek bir Z seviyesinde slice
     * @param mesh Mesh
     * @param z Z yüksekliği
     * @return Layer
     */
    Layer sliceAtZ(const mesh::Mesh& mesh, float z);

    /**
     * @brief Triangle ile düzlemin kesişimi
     * @param tri Triangle
     * @param z Z seviyesi
     * @param outSegment Çıkış line segment (varsa)
     * @return true = kesişim var
     */
    bool intersectTriangleWithPlane(const geometry::Triangle& tri,
                                    float z,
                                    LineSegment& outSegment);

    /**
     * @brief Mesh'in Z sınırlarını bul
     */
    void getBoundsZ(const mesh::Mesh& mesh, float& minZ, float& maxZ);
};

} // namespace slicing
} // namespace core

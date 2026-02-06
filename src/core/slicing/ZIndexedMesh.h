#pragma once

#include "core/geometry/triangle.h"
#include "core/mesh/mesh.h"
#include <vector>
#include <map>
#include <cmath>

namespace core {
namespace slicing {

/**
 * @brief Z-ekseninde indexlenmiş mesh
 *
 * Triangle'ları Z koordinatlarına göre bucket'lara ayırır.
 * Slicing sırasında sadece ilgili bucket'taki triangle'lar kontrol edilir.
 *
 * Performans: O(n*m) → O(n + m*k)
 * - n = triangle sayısı
 * - m = layer sayısı
 * - k = bucket başına ortalama triangle (~n/m)
 */
class ZIndexedMesh
{
public:
    /**
     * @brief Constructor - mesh'i indexle
     * @param mesh Kaynak mesh
     * @param bucketHeight Bucket yüksekliği (genelde layer height)
     */
    ZIndexedMesh(const mesh::Mesh& mesh, float bucketHeight);

    /**
     * @brief Belirli bir Z seviyesindeki triangle'ları getir
     * @param z Z koordinatı
     * @return İlgili triangle'lara pointer vector
     */
    std::vector<const geometry::Triangle*> getTrianglesAtZ(float z) const;

    /**
     * @brief İstatistikler
     */
    struct Stats
    {
        size_t totalBuckets = 0;
        size_t totalTriangles = 0;
        size_t totalReferences = 0;
        float avgTrianglesPerBucket = 0.0f;
        float minZ = 0.0f;
        float maxZ = 0.0f;
    };

    Stats getStats() const;

private:
    float m_bucketHeight;
    float m_minZ;
    float m_maxZ;

    // Bucket index → Triangle pointers
    std::map<int, std::vector<const geometry::Triangle*>> m_buckets;

    /**
     * @brief Z koordinatını bucket index'e çevir
     */
    int getBucketIndex(float z) const;

    /**
     * @brief Triangle'ın kapsadığı bucket aralığını bul
     */
    void getTriangleBucketRange(const geometry::Triangle& tri,
                                int& minBucket,
                                int& maxBucket) const;
};

} // namespace slicing
} // namespace core

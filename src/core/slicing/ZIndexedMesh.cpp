#include "ZIndexedMesh.h"
#include "SlicingConstants.h"
#include <algorithm>
#include <limits>

namespace core {
namespace slicing {

ZIndexedMesh::ZIndexedMesh(const mesh::Mesh& mesh, float bucketHeight)
    : m_bucketHeight(bucketHeight)
    , m_minZ(std::numeric_limits<float>::max())
    , m_maxZ(std::numeric_limits<float>::lowest())
{
    if (mesh.triangles.empty() || bucketHeight <= 0.0f)
    {
        return;
    }

    // 1. Z sınırlarını bul
    for (const auto& tri : mesh.triangles)
    {
        m_minZ = std::min({m_minZ, tri.vertex1.z, tri.vertex2.z, tri.vertex3.z});
        m_maxZ = std::max({m_maxZ, tri.vertex1.z, tri.vertex2.z, tri.vertex3.z});
    }

    // 2. Her triangle'ı ilgili bucket'lara ekle
    for (const auto& tri : mesh.triangles)
    {
        int minBucket, maxBucket;
        getTriangleBucketRange(tri, minBucket, maxBucket);

        // Triangle birden fazla bucket'a span edebilir
        for (int b = minBucket; b <= maxBucket; ++b)
        {
            m_buckets[b].push_back(&tri);
        }
    }

    // 3. Her bucket'ı optimize et
    for (auto& [bucket, triangles] : m_buckets)
    {
        triangles.shrink_to_fit();
    }
}

std::vector<const geometry::Triangle*> ZIndexedMesh::getTrianglesAtZ(float z) const
{
    if (z < m_minZ - EPSILON || z > m_maxZ + EPSILON)
    {
        return {};
    }

    int bucketIdx = getBucketIndex(z);

    auto it = m_buckets.find(bucketIdx);
    if (it != m_buckets.end())
    {
        return it->second;
    }

    return {};
}

int ZIndexedMesh::getBucketIndex(float z) const
{
    if (m_bucketHeight <= EPSILON)
    {
        return 0;
    }
    return static_cast<int>(std::floor((z - m_minZ) / m_bucketHeight));
}

void ZIndexedMesh::getTriangleBucketRange(const geometry::Triangle& tri,
                                          int& minBucket,
                                          int& maxBucket) const
{
    float triMinZ = std::min({tri.vertex1.z, tri.vertex2.z, tri.vertex3.z});
    float triMaxZ = std::max({tri.vertex1.z, tri.vertex2.z, tri.vertex3.z});

    minBucket = getBucketIndex(triMinZ);
    maxBucket = getBucketIndex(triMaxZ);
}

ZIndexedMesh::Stats ZIndexedMesh::getStats() const
{
    Stats stats;
    stats.totalBuckets = m_buckets.size();
    stats.minZ = m_minZ;
    stats.maxZ = m_maxZ;

    size_t totalTriangleRefs = 0;
    for (const auto& [bucket, triangles] : m_buckets)
    {
        totalTriangleRefs += triangles.size();
    }

    stats.totalReferences = totalTriangleRefs;
    stats.avgTrianglesPerBucket = stats.totalBuckets > 0
        ? static_cast<float>(totalTriangleRefs) / stats.totalBuckets
        : 0.0f;

    return stats;
}

} // namespace slicing
} // namespace core

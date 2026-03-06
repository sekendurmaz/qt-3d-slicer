#pragma once

#include <cstdint>
#include <string>
#include "core/mesh/mesh.h"

namespace io {
namespace loading {
namespace cache {

// Cache file header
struct MeshCacheHeader {
    uint32_t magic;           // 'MESH' = 0x4853454D
    uint32_t version;         // 1
    uint64_t timestamp;       // Original file modification time
    uint32_t vertexCount;     // Number of vertices
    uint32_t triangleCount;   // Number of triangles
};

class MeshCache {
public:
    // Check if cache file exists and is valid
    static bool isCacheValid(const std::string& originalFilePath);

    // Get cache file path from original file path
    static std::string getCachePath(const std::string& originalFilePath);

    // Save mesh to cache
    static bool saveCache(const std::string& originalFilePath, const core::mesh::Mesh& mesh);

    // Load mesh from cache
    static core::mesh::Mesh loadCache(const std::string& cacheFilePath);

private:
    static constexpr uint32_t CACHE_MAGIC = 0x4853454D;  // 'MESH'
    static constexpr uint32_t CACHE_VERSION = 1;

    static uint64_t getFileModificationTime(const std::string& filepath);
};

} // namespace cache
} // namespace loading
} // namespace io

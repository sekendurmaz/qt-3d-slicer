#include "MeshCache.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace io {
namespace loading {
namespace cache {

std::string MeshCache::getCachePath(const std::string& originalFilePath)
{
    return originalFilePath + ".cache";
}

uint64_t MeshCache::getFileModificationTime(const std::string& filepath)
{
    try {
        auto ftime = std::filesystem::last_write_time(filepath);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
        return std::chrono::system_clock::to_time_t(sctp);
    }
    catch (...) {
        return 0;
    }
}

bool MeshCache::isCacheValid(const std::string& originalFilePath)
{
    std::string cachePath = getCachePath(originalFilePath);

    // Check if cache file exists
    if (!std::filesystem::exists(cachePath)) {
        return false;
    }

    // Check if original file exists
    if (!std::filesystem::exists(originalFilePath)) {
        return false;
    }

    // Get modification times
    uint64_t originalTime = getFileModificationTime(originalFilePath);
    uint64_t cacheTime = getFileModificationTime(cachePath);

    // Cache must be newer than original
    if (cacheTime < originalTime) {
        std::cout << "  ⚠️  Cache outdated, will regenerate" << std::endl;
        return false;
    }

    // Read cache header to validate
    std::ifstream file(cachePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    MeshCacheHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.magic != CACHE_MAGIC) {
        std::cout << "  ❌ Invalid cache magic" << std::endl;
        return false;
    }

    if (header.version != CACHE_VERSION) {
        std::cout << "  ❌ Cache version mismatch" << std::endl;
        return false;
    }

    std::cout << "  ✅ Cache valid!" << std::endl;
    return true;
}

bool MeshCache::saveCache(const std::string& originalFilePath, const core::mesh::Mesh& mesh)
{
    std::string cachePath = getCachePath(originalFilePath);

    std::cout << "  💾 Saving cache: " << cachePath << std::endl;

    auto saveStart = std::chrono::high_resolution_clock::now();

    std::ofstream file(cachePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "  ❌ Failed to create cache file" << std::endl;
        return false;
    }

    // Write header
    MeshCacheHeader header;
    header.magic = CACHE_MAGIC;
    header.version = CACHE_VERSION;
    header.timestamp = getFileModificationTime(originalFilePath);
    header.vertexCount = 0;  // Not used (vertices are in triangles)
    header.triangleCount = static_cast<uint32_t>(mesh.triangles.size());

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // Write triangles (contains vertices v0, v1, v2)
    if (!mesh.triangles.empty()) {
        file.write(
            reinterpret_cast<const char*>(mesh.triangles.data()),
            mesh.triangles.size() * sizeof(core::geometry::Triangle)
            );
    }

    file.close();

    auto saveEnd = std::chrono::high_resolution_clock::now();
    auto saveMs = std::chrono::duration_cast<std::chrono::milliseconds>(saveEnd - saveStart).count();

    std::cout << "  ✅ Cache saved in " << saveMs << " ms" << std::endl;

    return true;
}

core::mesh::Mesh MeshCache::loadCache(const std::string& cacheFilePath)
{
    std::cout << "  📂 Loading from cache: " << cacheFilePath << std::endl;

    auto loadStart = std::chrono::high_resolution_clock::now();

    std::ifstream file(cacheFilePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open cache file");
    }

    // Read header
    MeshCacheHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.magic != CACHE_MAGIC) {
        throw std::runtime_error("Invalid cache file magic");
    }

    if (header.version != CACHE_VERSION) {
        throw std::runtime_error("Cache version mismatch");
    }

    // Create mesh
    core::mesh::Mesh mesh;
    mesh.triangles.resize(header.triangleCount);

    // Read triangles
    if (header.triangleCount > 0) {
        file.read(
            reinterpret_cast<char*>(mesh.triangles.data()),
            header.triangleCount * sizeof(core::geometry::Triangle)
            );
    }

    file.close();

    auto loadEnd = std::chrono::high_resolution_clock::now();
    auto loadMs = std::chrono::duration_cast<std::chrono::milliseconds>(loadEnd - loadStart).count();

    std::cout << "  ✅ Cache loaded in " << loadMs << " ms ("
              << header.triangleCount << " triangles)" << std::endl;

    return mesh;
}

} // namespace cache
} // namespace loading
} // namespace io

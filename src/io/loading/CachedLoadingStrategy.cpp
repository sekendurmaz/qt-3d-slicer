#include "CachedLoadingStrategy.h"
#include "cache/MeshCache.h"
#include <iostream>

namespace io {
namespace loading {

CachedLoadingStrategy::CachedLoadingStrategy(std::unique_ptr<ILoadingStrategy> wrappedStrategy)
    : m_wrappedStrategy(std::move(wrappedStrategy))
{
}

void CachedLoadingStrategy::load(
    const std::string& filepath,
    ProgressCallback onProgress,
    CompletionCallback onComplete)
{
    // Check if cache is valid
    if (cache::MeshCache::isCacheValid(filepath))
    {
        // Load from cache (FAST!)
        try
        {
            std::string cachePath = cache::MeshCache::getCachePath(filepath);

            if (onProgress)
                onProgress(50);

            core::mesh::Mesh mesh = cache::MeshCache::loadCache(cachePath);

            if (onProgress)
                onProgress(100);

            if (onComplete)
                onComplete(std::move(mesh), true, "");

            return;
        }
        catch (const std::exception& e)
        {
            std::cerr << "  ⚠️  Cache load failed: " << e.what() << std::endl;
            std::cerr << "  🔄 Falling back to original file..." << std::endl;
        }
    }

    // Cache miss or invalid - load from original file
    std::cout << "  🔄 Cache miss - loading original file..." << std::endl;

    m_wrappedStrategy->load(
        filepath,
        onProgress,
        [filepath, onComplete](core::mesh::Mesh mesh, bool success, std::string error) {
            if (!success)
            {
                if (onComplete)
                    onComplete(std::move(mesh), false, error);
                return;
            }

            // Save to cache for next time
            cache::MeshCache::saveCache(filepath, mesh);

            // Return the loaded mesh
            if (onComplete)
                onComplete(std::move(mesh), true, "");
        }
    );
}

} // namespace loading
} // namespace io

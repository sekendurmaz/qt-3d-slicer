#pragma once

#include "ILoadingStrategy.h"
#include <memory>

namespace io {
namespace loading {

/**
 * @brief Cached loading strategy with automatic cache management
 *
 * Wraps another loading strategy and adds caching:
 * - First load: Uses wrapped strategy + saves cache
 * - Subsequent loads: Loads from cache (10-30x faster!)
 * - Auto invalidation: Regenerates if source file changes
 */
class CachedLoadingStrategy : public ILoadingStrategy
{
public:
    /**
     * @brief Constructor
     * @param wrappedStrategy The underlying strategy to use for initial loads
     */
    explicit CachedLoadingStrategy(std::unique_ptr<ILoadingStrategy> wrappedStrategy);

    /**
     * @brief Load mesh with caching
     * @param filepath Path to the model file
     * @param onProgress Progress callback (0-100)
     * @param onComplete Completion callback
     */
    void load(
        const std::string& filepath,
        ProgressCallback onProgress,
        CompletionCallback onComplete
        ) override;

    // ILoadingStrategy interface implementation
    std::string name() const override { return "Cached " + m_wrappedStrategy->name(); }
    std::string description() const override { return "Cached version of " + m_wrappedStrategy->description(); }
    bool isAsync() const override { return m_wrappedStrategy->isAsync(); }
    std::string recommendedFor() const override { return m_wrappedStrategy->recommendedFor(); }

private:
    std::unique_ptr<ILoadingStrategy> m_wrappedStrategy;
};

} // namespace loading
} // namespace io

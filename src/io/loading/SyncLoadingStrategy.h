#pragma once

#include "ILoadingStrategy.h"

namespace io {
namespace loading {

/**
 * @brief Synchronous (blocking) loading strategy
 * Loads models immediately on the main thread
 */
class SyncLoadingStrategy : public ILoadingStrategy
{
public:
    std::string name() const override { return "Synchronous"; }
    std::string description() const override { return "Immediate blocking load"; }
    
    void load(
        const std::string& filepath,
        ProgressCallback onProgress,
        CompletionCallback onComplete
    ) override;
    
    bool isAsync() const override { return false; }
    std::string recommendedFor() const override { return "Small files (<10MB)"; }
};

} // namespace loading
} // namespace io
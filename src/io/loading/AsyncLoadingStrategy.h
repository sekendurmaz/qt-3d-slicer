#pragma once

#include "ILoadingStrategy.h"
#include <thread>
#include <atomic>

namespace io {
namespace loading {

/**
 * @brief Asynchronous (non-blocking) loading strategy
 * Loads models in a background thread
 */
class AsyncLoadingStrategy : public ILoadingStrategy
{
public:
    AsyncLoadingStrategy();
    ~AsyncLoadingStrategy();

    std::string name() const override { return "Asynchronous"; }
    std::string description() const override { return "Background thread loading"; }

    void load(
        const std::string& filepath,
        ProgressCallback onProgress,
        CompletionCallback onComplete
        ) override;

    bool supportsCancel() const override { return true; }
    void cancel() override;

    bool isAsync() const override { return true; }
    std::string recommendedFor() const override { return "Large files (>10MB)"; }

private:
    std::atomic<bool> m_cancelRequested{false};
    std::thread m_workerThread;

    void waitForCompletion();
};

} // namespace loading
} // namespace io

#include "AsyncLoadingStrategy.h"
#include "io/models/common/ModelFactory.h"
#include <stdexcept>
#include <iostream>  // ← DEĞİŞTİ!
#include <chrono>  // ← EKLE!

using namespace std::chrono;  // ← EKLE!
namespace io {
namespace loading {

AsyncLoadingStrategy::AsyncLoadingStrategy()
{
}

AsyncLoadingStrategy::~AsyncLoadingStrategy()
{
    cancel();
    waitForCompletion();
}

void AsyncLoadingStrategy::load(
    const std::string& filepath,
    ProgressCallback onProgress,
    CompletionCallback onComplete)
{
    cancel();
    waitForCompletion();

    m_cancelRequested = false;

    m_workerThread = std::thread([this, filepath, onProgress, onComplete]() {
        try
        {
            auto totalStart = std::chrono::high_resolution_clock::now();

            if (onProgress)
                onProgress(10);

            if (m_cancelRequested) {
                if (onComplete)
                    onComplete(core::mesh::Mesh(), false, "Cancelled by user");
                return;
            }

            if (onProgress)
                onProgress(30);

            // ⏱️ MODEL LOADING TIMER
            auto loadStart = std::chrono::high_resolution_clock::now();

            io::models::ModelFactory factory;
            core::mesh::Mesh mesh = factory.loadModel(filepath);

            auto loadEnd = std::chrono::high_resolution_clock::now();
            auto loadMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                              loadEnd - loadStart
                              ).count();

            std::cout << "  📂 File read + parse: " << loadMs << " ms" << std::endl;

            if (m_cancelRequested) {
                if (onComplete)
                    onComplete(core::mesh::Mesh(), false, "Cancelled by user");
                return;
            }

            if (onProgress)
                onProgress(100);

            auto totalEnd = std::chrono::high_resolution_clock::now();
            auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                               totalEnd - totalStart
                               ).count();

            std::cout << "  📊 Total async overhead: " << (totalMs - loadMs) << " ms" << std::endl;

            if (onComplete)
                onComplete(std::move(mesh), true, "");
        }
        catch (const std::exception& e)
        {
            if (onComplete)
                onComplete(core::mesh::Mesh(), false, e.what());
        }
    });
}

void AsyncLoadingStrategy::cancel()
{
    m_cancelRequested = true;
}

void AsyncLoadingStrategy::waitForCompletion()
{
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}



} // namespace loading
} // namespace io

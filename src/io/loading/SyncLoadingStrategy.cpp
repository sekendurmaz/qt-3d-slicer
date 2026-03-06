#include "SyncLoadingStrategy.h"
#include "io/models/common/ModelFactory.h"
#include <stdexcept>

namespace io {
namespace loading {

void SyncLoadingStrategy::load(
    const std::string& filepath,
    ProgressCallback onProgress,
    CompletionCallback onComplete)
{
    try
    {
        if (onProgress)
            onProgress(10);

        io::models::ModelFactory factory;
        core::mesh::Mesh mesh = factory.loadModel(filepath);

        if (onProgress)
            onProgress(100);

        if (onComplete)
            onComplete(std::move(mesh), true, "");
    }
    catch (const std::exception& e)
    {
        if (onComplete)
            onComplete(core::mesh::Mesh(), false, e.what());
    }
}

} // namespace loading
} // namespace io

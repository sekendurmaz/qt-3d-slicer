#pragma once

#include <string>
#include <functional>
#include "core/mesh/mesh.h"

namespace io {
namespace loading {

// Callback türleri
using ProgressCallback = std::function<void(int percentage)>;
using CompletionCallback = std::function<void(core::mesh::Mesh mesh, bool success, std::string error)>;

/**
 * @brief Loading strategy interface
 */
class ILoadingStrategy
{
public:
    virtual ~ILoadingStrategy() = default;

    virtual std::string name() const = 0;
    virtual std::string description() const = 0;

    virtual void load(
        const std::string& filepath,
        ProgressCallback onProgress,
        CompletionCallback onComplete
        ) = 0;

    virtual bool supportsCancel() const { return false; }
    virtual void cancel() {}

    virtual bool isAsync() const = 0;
    virtual std::string recommendedFor() const = 0;
};

} // namespace loading
} // namespace io

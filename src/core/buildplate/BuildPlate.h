#pragma once

#include "core/geometry/vec3.h"
#include "core/geometry/aabb.h"
#include <string>

namespace core {
namespace buildplate {

enum class PlateType
{
    Rectangular,
    Circular
};

class BuildPlate
{
public:
    virtual ~BuildPlate() = default;

    virtual PlateType type() const = 0;
    virtual std::string name() const = 0;

    // Dimensions
    virtual float width() const = 0;
    virtual float depth() const = 0;
    virtual float height() const = 0;

    // Geometry
    virtual geometry::AABB bounds() const = 0;
    virtual bool contains(const geometry::Vec3& point) const = 0;

    // Grid rendering info
    virtual float gridSpacing() const { return 10.0f; }

protected:
    BuildPlate() = default;
};

} // namespace buildplate
} // namespace core

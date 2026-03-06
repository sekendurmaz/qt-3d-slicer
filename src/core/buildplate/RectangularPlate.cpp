#include "RectangularPlate.h"

namespace core {
namespace buildplate {

RectangularPlate::RectangularPlate(float width, float depth, float height)
    : width_(width)
    , depth_(depth)
    , height_(height)
{
}

geometry::AABB RectangularPlate::bounds() const
{
    geometry::AABB aabb;
    aabb.min = geometry::Vec3(-width_ / 2.0f, -depth_ / 2.0f, 0.0f);
    aabb.max = geometry::Vec3(width_ / 2.0f, depth_ / 2.0f, height_);
    return aabb;
}

bool RectangularPlate::contains(const geometry::Vec3& point) const
{
    float halfWidth = width_ / 2.0f;
    float halfDepth = depth_ / 2.0f;

    return point.x >= -halfWidth && point.x <= halfWidth &&
           point.y >= -halfDepth && point.y <= halfDepth &&
           point.z >= 0.0f && point.z <= height_;
}

} // namespace buildplate
} // namespace core

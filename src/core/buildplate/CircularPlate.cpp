#include "CircularPlate.h"

namespace core {
namespace buildplate {

CircularPlate::CircularPlate(float diameter, float height)
    : diameter_(diameter)
    , height_(height)
{
}

geometry::AABB CircularPlate::bounds() const
{
    float r = radius();
    geometry::AABB aabb;
    aabb.min = geometry::Vec3(-r, -r, 0.0f);
    aabb.max = geometry::Vec3(r, r, height_);
    return aabb;
}

bool CircularPlate::contains(const geometry::Vec3& point) const
{
    float r = radius();
    float distXY = std::sqrt(point.x * point.x + point.y * point.y);

    return distXY <= r &&
           point.z >= 0.0f &&
           point.z <= height_;
}

} // namespace buildplate
} // namespace core

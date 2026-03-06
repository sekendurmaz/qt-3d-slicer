#pragma once

#include "BuildPlate.h"

namespace core {
namespace buildplate {

class RectangularPlate : public BuildPlate
{
public:
    RectangularPlate(float width, float depth, float height);

    PlateType type() const override { return PlateType::Rectangular; }
    std::string name() const override { return "Rectangular Build Plate"; }

    float width() const override { return width_; }
    float depth() const override { return depth_; }
    float height() const override { return height_; }

    geometry::AABB bounds() const override;
    bool contains(const geometry::Vec3& point) const override;

private:
    float width_;
    float depth_;
    float height_;
};

} // namespace buildplate
} // namespace core

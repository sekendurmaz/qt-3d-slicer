#pragma once

#include "BuildPlate.h"
#include <cmath>

namespace core {
namespace buildplate {

class CircularPlate : public BuildPlate
{
public:
    CircularPlate(float diameter, float height);

    PlateType type() const override { return PlateType::Circular; }
    std::string name() const override { return "Circular Build Plate"; }

    float width() const override { return diameter_; }
    float depth() const override { return diameter_; }
    float height() const override { return height_; }

    float diameter() const { return diameter_; }
    float radius() const { return diameter_ / 2.0f; }

    geometry::AABB bounds() const override;
    bool contains(const geometry::Vec3& point) const override;

private:
    float diameter_;
    float height_;
};

} // namespace buildplate
} // namespace core

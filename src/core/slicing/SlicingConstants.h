#pragma once

namespace core {
namespace slicing {

// Numerical constants
constexpr float EPSILON = 1e-6f;           // Floating point tolerance
constexpr float EPSILON_SQ = EPSILON * EPSILON;

// Sanity limits
constexpr int MAX_LAYER_COUNT = 100000;    // Safety limit
constexpr float MIN_LAYER_HEIGHT = 0.01f;  // 0.01mm
constexpr float MAX_LAYER_HEIGHT = 10.0f;  // 10mm

} // namespace slicing
} // namespace core

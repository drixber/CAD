#pragma once

#include "math/Vector3.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace math {

constexpr double kDistanceTolerance = 1e-7;
constexpr double kAngularTolerance  = 1e-9;
constexpr double kParamTolerance    = 1e-9;

inline bool pointsEqual(const Point3& a, const Point3& b) {
    const double dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return (dx*dx + dy*dy + dz*dz) <= kDistanceTolerance * kDistanceTolerance;
}

inline bool vectorsParallel(const Vector3& a, const Vector3& b) {
    const double la = a.length(), lb = b.length();
    if (la <= kDistanceTolerance || lb <= kDistanceTolerance) return true;
    const double d = std::abs(a.dot(b) / (la * lb));
    return d >= (1.0 - kAngularTolerance);
}

}  // namespace math
}  // namespace kernel
}  // namespace cad

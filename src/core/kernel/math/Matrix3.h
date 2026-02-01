#pragma once

#include "math/Vector3.h"
#include <cmath>

namespace cad {
namespace kernel {
namespace math {

struct Matrix3 {
    double m[3][3]{{1,0,0},{0,1,0},{0,0,1}};

    Matrix3() = default;

    static Matrix3 rotationX(double angleRad) {
        const double c = std::cos(angleRad), s = std::sin(angleRad);
        Matrix3 r;
        r.m[1][1] = c; r.m[1][2] = -s;
        r.m[2][1] = s; r.m[2][2] = c;
        return r;
    }
    static Matrix3 rotationY(double angleRad) {
        const double c = std::cos(angleRad), s = std::sin(angleRad);
        Matrix3 r;
        r.m[0][0] = c; r.m[0][2] = s;
        r.m[2][0] = -s; r.m[2][2] = c;
        return r;
    }
    static Matrix3 rotationZ(double angleRad) {
        const double c = std::cos(angleRad), s = std::sin(angleRad);
        Matrix3 r;
        r.m[0][0] = c; r.m[0][1] = -s;
        r.m[1][0] = s; r.m[1][1] = c;
        return r;
    }
    static Matrix3 scale(double sx, double sy, double sz) {
        Matrix3 r;
        r.m[0][0] = sx; r.m[1][1] = sy; r.m[2][2] = sz;
        return r;
    }

    Vector3 apply(const Vector3& v) const {
        return Vector3(
            m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
            m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
            m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z
        );
    }
};

}  // namespace math
}  // namespace kernel
}  // namespace cad

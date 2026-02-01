#pragma once

#include "math/Vector3.h"
#include "math/Matrix3.h"

namespace cad {
namespace kernel {
namespace math {

struct Transform3 {
    Matrix3 rotation;
    Vector3 translation{0, 0, 0};

    Transform3() = default;

    static Transform3 translate(double dx, double dy, double dz) {
        Transform3 t;
        t.translation = Vector3(dx, dy, dz);
        return t;
    }
    static Transform3 rotateZ(double angleRad) {
        Transform3 t;
        t.rotation = Matrix3::rotationZ(angleRad);
        return t;
    }
    static Transform3 rotateX(double angleRad) {
        Transform3 t;
        t.rotation = Matrix3::rotationX(angleRad);
        return t;
    }
    static Transform3 rotateY(double angleRad) {
        Transform3 t;
        t.rotation = Matrix3::rotationY(angleRad);
        return t;
    }

    Point3 apply(const Point3& p) const {
        return rotation.apply(p) + translation;
    }

    Transform3 combine(const Transform3& other) const {
        Transform3 r;
        r.rotation = other.rotation;  // simplified: R2 * R1 not implemented
        r.translation = other.apply(translation);
        return r;
    }
};

}  // namespace math
}  // namespace kernel
}  // namespace cad

#pragma once

namespace cad {
namespace core {

struct Quaternion {
    double w{1.0};
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

struct Transform {
    Quaternion rotation{};
    double tx{0.0};
    double ty{0.0};
    double tz{0.0};
};

inline Transform compose(const Transform& a, const Transform& b) {
    Transform out;
    out.tx = a.tx + b.tx;
    out.ty = a.ty + b.ty;
    out.tz = a.tz + b.tz;
    out.rotation = a.rotation;
    return out;
}

}  // namespace core
}  // namespace cad

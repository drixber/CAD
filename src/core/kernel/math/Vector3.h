#pragma once

#include <cmath>

namespace cad {
namespace kernel {
namespace math {

struct Vector3 {
    double x{0.0};
    double y{0.0};
    double z{0.0};

    Vector3() = default;
    Vector3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3 normalized() const {
        const double len = length();
        if (len <= 0.0) return *this;
        return Vector3(x / len, y / len, z / len);
    }

    double dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 cross(const Vector3& other) const {
        return Vector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
    Vector3 operator*(double s) const {
        return Vector3(x * s, y * s, z * s);
    }
};

inline Vector3 operator*(double s, const Vector3& v) {
    return v * s;
}

using Point3 = Vector3;

}  // namespace math
}  // namespace kernel
}  // namespace cad

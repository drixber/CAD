#pragma once

#include <cmath>

namespace cad {
namespace kernel {
namespace math {

struct Vector2 {
    double x{0.0};
    double y{0.0};

    Vector2() = default;
    Vector2(double x_, double y_) : x(x_), y(y_) {}

    double length() const {
        return std::sqrt(x * x + y * y);
    }

    Vector2 normalized() const {
        const double len = length();
        if (len <= 0.0) return *this;
        return Vector2(x / len, y / len);
    }

    double dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }

    double angleTo(const Vector2& other) const {
        const double d = dot(other);
        const double lenProd = length() * other.length();
        if (lenProd <= 0.0) return 0.0;
        const double cosAngle = std::max(-1.0, std::min(1.0, d / lenProd));
        return std::acos(cosAngle);
    }

    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }
    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }
    Vector2 operator*(double s) const {
        return Vector2(x * s, y * s);
    }
};

inline Vector2 operator*(double s, const Vector2& v) {
    return v * s;
}

using Point2 = Vector2;

}  // namespace math
}  // namespace kernel
}  // namespace cad

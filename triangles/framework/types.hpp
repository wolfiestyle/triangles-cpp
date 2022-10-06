#ifndef __FRAMEWORK_TYPES_HPP
#define __FRAMEWORK_TYPES_HPP

#include <iostream>

struct Point2d {
    float x, y;
};

template <typename T>
struct Vec3 {
    T x, y, z;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, Vec3<T> const& c) {
    os << "[" << c.x << ", " << c.y << ", " << c.z << "]";
    return os;
}

template <typename T>
struct ColorRGBA {
    T r, g, b, a;

    ColorRGBA<T> operator+(ColorRGBA const& rhs) {
        return ColorRGBA {r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a};
    }

    ColorRGBA<T> operator/(float rhs) {
        return ColorRGBA { r / rhs, g / rhs, b / rhs,  a / rhs };
    }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, ColorRGBA<T> const& c) {
    os << "[" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << "]";
    return os;
}

#endif // __FRAMEWORK_TYPES_HPP

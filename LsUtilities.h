#pragma once
#include <iosfwd>
#include <cmath>

constexpr double LsEpsilon = 0.001;

template<typename T>
struct LsVec2 {
    T x, y;

    LsVec2& operator = (const LsVec2& other) = default;
    LsVec2 operator + (const LsVec2& other) { x += other.x; y += other.y; return *this; }
    LsVec2 operator - (const LsVec2& other) { x -= other.x; y -= other.y; return *this; }
    LsVec2 operator * (const LsVec2& other) { x *= other.x; y *= other.y; return *this; }
    LsVec2 operator / (const LsVec2& other) { x /= other.x; y /= other.y; return *this; }
    LsVec2 operator += (const LsVec2& other) { x += other.x; y += other.y; return *this; }
    LsVec2 operator -= (const LsVec2& other) { x -= other.x; y -= other.y; return *this; }
    LsVec2 operator *= (const LsVec2& other) { x *= other.x; y *= other.y; return *this; }
    LsVec2 operator /= (const LsVec2& other) { x /= other.x; y /= other.y; return *this; }
    bool operator == (const LsVec2& other) { return x == other.x && y == other.y; }
    bool operator != (const LsVec2& other) { return x != other.x && y != other.y; }
};
template<typename T>
std::ostream& operator << (std::ostream& os, const LsVec2<T>& v) { return os << v.x << " " << v.y; };

using LsVec2I = LsVec2<int>;
using LsVec2U = LsVec2<unsigned>;
using LsVec2F = LsVec2<float>;
using LsVec2D = LsVec2<double>;

template<typename T>
bool pointsAreEqual(const LsVec2<T>& a, const LsVec2<T>& b) {
    return static_cast<double>(std::abs(a.x - b.x)) < LsEpsilon && static_cast<double>(std::abs(a.y - b.y)) < LsEpsilon;
}
float distance(const LsVec2F& a, const LsVec2F& b);
double distance(const LsVec2D& a, const LsVec2D& b);
float angleTo(const LsVec2F& observer, const LsVec2F& target);
double angleTo(const LsVec2D& observer, const LsVec2D& target);


#include "LsUtilities.h"
#include <cmath>
#include <numbers>

double distance(const LsVec2D& a, const LsVec2D& b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

float distance(const LsVec2F& a, const LsVec2F& b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

double angleTo(const LsVec2D& observer, const LsVec2D& target) {
    return std::atan2(target.y - observer.y, target.x - observer.x)
           * (180.0 / M_PI);
}

float angleTo(const LsVec2F& observer, const LsVec2F& target) {
    return std::atan2(target.y - observer.y, target.x - observer.x)
           * (180.0f / static_cast<float>(M_PI));
}



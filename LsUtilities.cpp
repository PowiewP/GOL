#include "LsUtilities.h"
#include <cmath>

template<typename T>
bool pointsAreEqual(const LsVec2<T>& a, const LsVec2<T>& b) {
    return static_cast<double>(std::abs(a.x - b.x)) < LsEpsilon && static_cast<double>(std::abs(a.y - b.y)) < LsEpsilon;
}

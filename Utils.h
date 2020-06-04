#ifndef UTILS_DEFINED
#define UTILS_DEFINED

#include <algorithm>
int clamp(int n, int a, int b) {
    return std::max(a, std::min(b, n));
}
float clamp(float n, float a, float b) {
    return std::max(a, std::min(b, n));
}

#endif
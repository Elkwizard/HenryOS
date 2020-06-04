#ifndef UTILS_DEFINED
#define UTILS_DEFINED

#include <algorithm>
int clamp(int n, int a, int b) {
    return std::max(a, std::min(b, n));
}
float clamp(float n, float a, float b) {
    return std::max(a, std::min(b, n));
}
float remap(float n, float a, float b, float a2, float b2) {
    return (b2 - a2) * (n - a) / (b - a) + a2;
}

#endif
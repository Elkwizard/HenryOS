#ifndef RANDOM_INCLUDED
#define RANDOM_INCLUDED

#include <cmath>

class Interpolation {
    public:
        static float smoothT(float t) {
            return -2.0f * t * t * t + 3.0f * t * t;
        }
        static float slerp(float a, float b, float t) {
            return lerp(a, b, smoothT(t));
        }
        static float squadLerp(float a, float b, float c, float d, float tx, float ty) {
            return quadLerp(a, b, c, d, smoothT(tx), smoothT(ty));
        }
        static float lerp(float a, float b, float t) {
            return a * (1 - t) + b * t;
        }
        static float quadLerp(float a, float b, float c, float d, float tx, float ty) {
            float l = lerp(a, c, ty);
            float r = lerp(b, d, ty);
            float per = lerp(l, r, tx);
            return per;
        }
        static float cubeLerp(float a, float b, float c, float d, float a2, float b2, float c2, float d2, float tx, float ty, float tz) {
            float top = quadLerp(a, b, c, d, tx, ty);
            float bottom = quadLerp(a2, b2, c2, d2, tx, ty);
            return lerp(top, bottom, tz);
        }
};
class Random {
    public:
        static float seed;
        static float seedRand(float seed) {
            seed += 10000.0f;
            float a = fmod(seed * 6.12849, 8.7890975);
            float b = fmod(a * 256783945.4758903, 238462.567890);
            float r = fmod(a * b, 1.0f);
            return r;
        }
        static float noiseTCorrect(float t) {
            return Interpolation::smoothT(t);
        }
        static float s_p1D(float x, float seed) {
            return seedRand(seed + floor(x));
        }
        static float perlin(float x, float f = 1, float seed = Random::seed) {
            x *= f;
            return Interpolation::lerp(s_p1D(x, seed), s_p1D(x + 1, seed), fmod(x, 1.0f));
        }
        static float s_p2D(float x, float y, float seed) {
            return seedRand(floor(x) + floor(y) * 2000 + seed * 100000);
        }
        static float perlin2D(float x, float y, float f = 1, float seed = Random::seed) {
            x *= f;
            y *= f;
            return Interpolation::squadLerp(
                s_p2D(x, y, seed), s_p2D(x + 1, y, seed), s_p2D(x, y + 1, seed), s_p2D(x + 1, y + 1, seed), 
                fmod(x, 1.0f), fmod(y, 1.0f));
        }
        static float s_p3D (float x, float y, float z, float seed) {
            return seedRand(seedRand(floor(x)) + seedRand(floor(y) * 2000) + seedRand(floor(z) * 2000000) + seed * 100000);
        }
        static float perlin3D(float x, float y, float z, float f = 1, float seed = Random::seed) {
            x *= f;
            y *= f;
            z *= f;
            return Interpolation::cubeLerp(
                s_p3D(x, y, z, seed), s_p3D(x + 1, y, z, seed), s_p3D(x, y + 1, z, seed), s_p3D(x + 1, y + 1, z, seed),
                s_p3D(x, y, z + 1, seed), s_p3D(x + 1, y, z + 1, seed), s_p3D(x, y + 1, z + 1, seed), s_p3D(x + 1, y + 1, z + 1, seed),
                noiseTCorrect(fmod(x, 1.0f)), noiseTCorrect(fmod(y, 1.0f)), noiseTCorrect(fmod(z, 1.0f)));
        }
        struct VoronoiCell {
            float x = 0.0f, y = 0.0f, z = 0.0f;
        };
        static VoronoiCell getVoronoiCell(float x) {
            return { floor(x) + seedRand(floor(x)), 0, 0 };
        }
        static float voronoi(float x, float f = 1, float seed = Random::seed) {
            x *= f;
            x += seed;
            float bestDist = INFINITY;
            for (int i = -1; i < 2; i++) {
                VoronoiCell cell = getVoronoiCell(x + i);
                float dist = cell.x - x;
                dist = dist * dist;
                if (dist < bestDist) bestDist = dist;
            }
            return bestDist;
        }
        static VoronoiCell getVoronoiCell2D(float x, float y) {
            return { 
                floor(x) + seedRand(floor(x) + floor(y) * 1000.0f), 
                floor(y) + seedRand(floor(y) + floor(x) * 10000.0f),
                0
            };
        }
        static float voronoi2D(float x, float y, float f = 1, float seed = Random::seed) {
            x *= f;
            y *= f;
            x += seed;
            y += seed * 2000;
            float bestDist = INFINITY;
            for (int i = -1; i < 2; i++) for (int j = -1; j < 2; j++) {
                VoronoiCell cell = getVoronoiCell2D(x + i, y + j);
                float dx = cell.x - x;
                float dy = cell.y - y;
                float dist = dx * dx + dy * dy;
                if (dist < bestDist) bestDist = dist;
            }
            return bestDist;
        }
        static VoronoiCell getVoronoiCell3D(float x, float y, float z) {
            return {
                floor(x) + seedRand(floor(x) + floor(y) * 1000 + floor(z) * 10000),
                floor(y) + seedRand(floor(y) + floor(x) * 1000000 + floor(z) * 900),
                floor(z) + seedRand(floor(y) * 10000 + floor(x) * 100 + floor(z) * 90000)
            };
        }
        static float voronoi3D(float x, float y, float z, float f = 1, float seed = Random::seed) {
            x *= f;
            y *= f;
            z *= f;
            x += seed;
            y += seed * 2000;
            z += seed * 2000000;
            float bestDist = INFINITY;
            for (int i = -1; i < 2; i++) for (int j = -1; j < 2; j++) for (int k = -1; k < 2; k++) {
                VoronoiCell cell = getVoronoiCell3D(x + i, y + j, z + k);
                float dx = cell.x - x;
                float dy = cell.y - y;
                float dz = cell.z - z;
                float dist = dx * dx + dy * dy + dz * dz;
                if (dist < bestDist) bestDist = dist;
            }
            return bestDist;
        }
};
float Random::seed = 0;

#endif
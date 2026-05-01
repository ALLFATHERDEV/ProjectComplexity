#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <random>
#include <vector>

class PerlinNoise {
public:
    explicit PerlinNoise(uint32_t seed) : m_Permutation(512) {
        std::iota(m_Permutation.begin(), m_Permutation.begin() + 256, 0);

        std::mt19937 generator(seed);
        std::shuffle(m_Permutation.begin(), m_Permutation.begin() + 256, generator);

        for (int i = 0; i < 256; i++) {
            m_Permutation[256 + i] = m_Permutation[i];
        }
    }

    double noise(double x, double y) const {
        const int xi = static_cast<int>(std::floor(x)) & 255;
        const int yi = static_cast<int>(std::floor(y)) & 255;

        const double xf = x - std::floor(x);
        const double yf = y - std::floor(y);

        const double u = fade(xf);
        const double v = fade(yf);

        const int aa = m_Permutation[m_Permutation[xi] + yi];
        const int ab = m_Permutation[m_Permutation[xi] + yi + 1];
        const int ba = m_Permutation[m_Permutation[xi + 1] + yi];
        const int bb = m_Permutation[m_Permutation[xi + 1] + yi + 1];

        const double x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1.0, yf), u);
        const double x2 = lerp(grad(ab, xf, yf - 1.0), grad(bb, xf - 1.0, yf - 1.0), u);

        return lerp(x1, x2, v);
    }

    double octaveNoise(double x, double y, int octaves, double persistence) const {
        double total = 0.0;
        double frequency = 1.0;
        double amplitude = 1.0;
        double maxValue = 0.0;

        for (int i = 0; i < octaves; i++) {
            total += noise(x * frequency, y * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= 2.0;
        }

        return maxValue > 0.0 ? total / maxValue : 0.0;
    }

private:
    static double fade(double t) {
        return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    }

    static double lerp(double a, double b, double t) {
        return a + t * (b - a);
    }

    static double grad(int hash, double x, double y) {
        switch (hash & 3) {
            case 0:
                return x + y;
            case 1:
                return -x + y;
            case 2:
                return x - y;
            default:
                return -x - y;
        }
    }

    std::vector<int> m_Permutation;
};

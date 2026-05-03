#pragma once
#include <random>

namespace FootballManager {

    class Utils {
    public:
        static std::mt19937 rng;
        static void initRNG();
        static int randInt(int min, int max);
        static double randDouble(double min, double max);
    };

} // namespace FootballManager
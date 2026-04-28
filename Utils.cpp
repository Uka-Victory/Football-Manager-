#include "Utils.hpp"
#include <chrono>

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

int randInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

double randDouble(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}
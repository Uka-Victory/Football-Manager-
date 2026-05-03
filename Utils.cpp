#include "Utils.hpp"

namespace FootballManager {

    std::mt19937 Utils::rng;

    void Utils::initRNG() {
        std::random_device rd;
        rng.seed(rd());
    }

    int Utils::randInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }

    double Utils::randDouble(double min, double max) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(rng);
    }

} // namespace FootballManager
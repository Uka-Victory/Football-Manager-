// Utils.cpp
#include "Utils.hpp"
#include <algorithm>

std::mt19937& Utils::getRng() {
    static std::mt19937 rng(std::random_device{}());
    return rng;
}

void Utils::initRNG() { getRng(); }

int Utils::randInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(getRng());
}

double Utils::randDouble() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(getRng());
}

std::string Utils::randomPosition() {
    static const std::vector<std::string> positions = {
        "GK", "CB", "LB", "RB", "DM", "CM", "LM", "RM", "AM", "LW", "RW", "ST"
    };
    return positions[randInt(0, positions.size() - 1)];
}

int Utils::nextPlayerId = 1000;
int Utils::generateUniquePlayerId() { return nextPlayerId++; }

size_t Utils::weightedRandomIndex(const std::vector<int>& weights) {
    if (weights.empty()) return 0;
    int total = 0;
    for (int w : weights) total += w;
    if (total <= 0) return randInt(0, weights.size() - 1);
    int r = randInt(0, total - 1);
    int cumul = 0;
    for (size_t i = 0; i < weights.size(); ++i) {
        cumul += weights[i];
        if (r < cumul) return i;
    }
    return weights.size() - 1;
}
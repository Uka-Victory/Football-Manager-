#include "Utils.hpp"
#include <algorithm>

static std::mt19937 rng;

void Utils::initRNG() {
    std::random_device rd;
    rng.seed(rd());
}

int Utils::randInt(int min, int max) {
    if (min > max) std::swap(min, max);
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

double Utils::randDouble() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

std::string Utils::generateUniqueId(const std::string& prefix) {
    const char hex_chars[] = "0123456789ABCDEF";
    std::string id = prefix;
    for (int i = 0; i < 8; ++i) {
        id += hex_chars[randInt(0, 15)];
    }
    return id;
}

std::string Utils::randomPosition() {
    static const std::vector<std::string> positions = {
        "GK", "CB", "LB", "RB", "DM", "CM", "LM", "RM", "AM", "LW", "RW", "ST"
    };
    return positions[randInt(0, positions.size() - 1)];
}

size_t Utils::weightedRandomIndex(const std::vector<int>& weights) {
    if (weights.empty()) return 0;
    int total = 0;
    for (int w : weights) total += w;
    if (total == 0) return randInt(0, static_cast<int>(weights.size()) - 1);
    int r = randInt(0, total - 1);
    int cumul = 0;
    for (size_t i = 0; i < weights.size(); ++i) {
        cumul += weights[i];
        if (r < cumul) return i;
    }
    return weights.size() - 1;
}
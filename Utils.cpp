#include "Utils.hpp"
#include <random>
#include <sstream>
#include <iomanip>

static std::mt19937 rng; // Mersenne Twister for high-quality randomness

void Utils::initRNG() {
    std::random_device rd;
    rng.seed(rd());
}

int Utils::randInt(int min, int max) {
    if (min > max) std::swap(min, max);
    std::uniform_int_distribution<int> dist(min, max);
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
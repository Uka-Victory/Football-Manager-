// Utils.hpp
#pragma once
#include <string>
#include <vector>
#include <random>

class Utils {
public:
    static void initRNG();
    static std::mt19937& getRng();
    static int randInt(int min, int max);
    static double randDouble();
    static std::string randomPosition();
    static int generateUniquePlayerId();
    static size_t weightedRandomIndex(const std::vector<int>& weights);
private:
    static int nextPlayerId;
};
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <random>

class Utils {
public:
    static void initRNG();
    static int randInt(int min, int max);
    static double randDouble();
    static std::string generateUniqueId(const std::string& prefix = "P_");
    static std::string randomPosition();
    static size_t weightedRandomIndex(const std::vector<int>& weights);
};

#endif
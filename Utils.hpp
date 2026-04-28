#ifndef UTILS_HPP
#define UTILS_HPP

#include <random>

extern std::mt19937 rng;

int randInt(int min, int max);
double randDouble(double min, double max);

#endif
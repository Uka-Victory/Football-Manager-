// NamePool.cpp
#include "NamePool.hpp"
#include "Utils.hpp"
#include <fstream>
#include <iostream>

bool NamePool::load(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        firstNames.push_back(line);
        std::getline(in, line);
        lastNames.push_back(line);
    }
    return !firstNames.empty();
}

std::string NamePool::generateFirstName(const std::string&) const {
    if (firstNames.empty()) return "John";
    return firstNames[Utils::randInt(0, firstNames.size()-1)];
}

std::string NamePool::generateLastName(const std::string&) const {
    if (lastNames.empty()) return "Doe";
    return lastNames[Utils::randInt(0, lastNames.size()-1)];
}

std::string NamePool::generateName(const std::string& country) const {
    return generateFirstName(country) + " " + generateLastName(country);
}
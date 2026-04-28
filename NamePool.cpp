#include "NamePool.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <chrono>
#include <iostream>

NamePool::NamePool() : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {}

bool NamePool::loadFromJson(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
        return false;
    }
    nlohmann::json data;
    file >> data;
    
    nameData.clear();
    countryIndex.clear();

    for (auto& [country, names] : data.items()) {
        if (!names.is_array()) continue;
        NationNames nn;
        nn.country = country;
        for (auto& name : names) {
            if (name.is_string()) {
                nn.firstNames.push_back(name.get<std::string>());
                nn.lastNames.push_back(name.get<std::string>()); // reuse for last names
            }
        }
        if (!nn.firstNames.empty()) {
            nameData.push_back(std::move(nn));
            countryIndex[country] = nameData.size() - 1;
        }
    }
    return true;
}

size_t NamePool::getIndex(const std::string& nationality) const {
    auto it = countryIndex.find(nationality);
    if (it != countryIndex.end()) return it->second;
    // fallback to first country if nationality not found
    return 0;
}

std::string NamePool::getRandomFirstName(const std::string& nationality) const {
    size_t idx = getIndex(nationality);
    if (idx >= nameData.size()) return "Unknown";
    auto& vec = nameData[idx].firstNames;
    if (vec.empty()) return "Unknown";
    std::uniform_int_distribution<size_t> dist(0, vec.size() - 1);
    return vec[dist(rng)];
}

std::string NamePool::getRandomLastName(const std::string& nationality) const {
    size_t idx = getIndex(nationality);
    if (idx >= nameData.size()) return "Unknown";
    auto& vec = nameData[idx].lastNames;
    if (vec.empty()) return "Unknown";
    std::uniform_int_distribution<size_t> dist(0, vec.size() - 1);
    return vec[dist(rng)];
}

std::string NamePool::getRandomFullName(const std::string& nationality) const {
    return getRandomFirstName(nationality) + " " + getRandomLastName(nationality);
}
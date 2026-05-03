#include "NamePool.hpp"
#include "Utils.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>

bool NamePool::loadFromJson(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    try {
        nlohmann::json j;
        file >> j;
        for (auto& [country, names] : j.items()) {
            NationNames nn;
            for (const auto& n : names) nn.names.push_back(n.get<std::string>());
            database[country] = nn;
        }
        return true;
    } catch (...) { return false; }
}

std::string NamePool::generateName(const std::string& nationality) {
    auto it = database.find(nationality);
    if (it == database.end() || it->second.names.empty()) return "Unknown Player";
    
    const auto& list = it->second.names;
    std::string first = list[Utils::randInt(0, list.size() - 1)];
    std::string last = list[Utils::randInt(0, list.size() - 1)];
    return first + " " + last;
}
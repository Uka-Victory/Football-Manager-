#ifndef NAMEPOOL_HPP
#define NAMEPOOL_HPP

#include <string>
#include <vector>
#include <unordered_map>

class NamePool {
private:
    struct NationNames {
        std::vector<std::string> names;
    };
    std::unordered_map<std::string, NationNames> database;

public:
    NamePool() = default;
    bool loadFromJson(const std::string& filepath);
    std::string generateName(const std::string& nationality);
};

#endif
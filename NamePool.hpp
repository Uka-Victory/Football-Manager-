#ifndef NAMEPOOL_HPP
#define NAMEPOOL_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <random>

class NamePool {
public:
    NamePool();
    bool loadFromJson(const std::string& filepath);
    std::string getRandomFirstName(const std::string& nationality) const;
    std::string getRandomLastName(const std::string& nationality) const;
    std::string getRandomFullName(const std::string& nationality) const;

private:
    struct NationNames {
        std::string country;
        std::vector<std::string> firstNames;
        std::vector<std::string> lastNames;   // same as firstNames for now
    };
    std::vector<NationNames> nameData;
    mutable std::mt19937 rng;
    std::unordered_map<std::string, size_t> countryIndex;

    size_t getIndex(const std::string& nationality) const;
};

#endif
// NamePool.hpp
#pragma once
#include <string>
#include <vector>

class NamePool {
public:
    bool load(const std::string& filename);
    std::string generateName(const std::string& country) const;
    std::string generateFirstName(const std::string& country) const;
    std::string generateLastName(const std::string& country) const;
private:
    std::vector<std::string> firstNames, lastNames;
};
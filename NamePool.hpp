#pragma once
#include <string>
#include <vector>

namespace FootballManager {

    class NamePool {
    private:
        std::vector<std::string> firstNames;
        std::vector<std::string> lastNames;
        std::vector<std::string> countries;

    public:
        NamePool();
        std::string generateName(const std::string& nation);
        std::string getRandomCountry();
    };

} // namespace FootballManager
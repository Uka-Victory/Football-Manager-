// WorldData.hpp
#pragma once
#include <string>
#include <vector>
#include "json.hpp"
using json = nlohmann::json;

struct TeamInfo {
    std::string name;
    int level;
    int seniorCount, youthCount;
    std::string country;
    std::string primaryColour, secondaryColour, stadium;
    int founded;
};

struct LeagueInfo {
    std::string name;
    int tier, level;
    int roundsPerOpponent, promotionSpots, relegationSpots;
    std::vector<TeamInfo> teams;
};

struct CountryInfo {
    std::string name;
    std::vector<LeagueInfo> leagues;
};

class WorldData {
public:
    bool load(const std::string& filename);
    const std::vector<CountryInfo>& getBaseCountries() const;
private:
    std::vector<CountryInfo> m_countries;
};
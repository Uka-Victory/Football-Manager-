#ifndef WORLDDATA_HPP
#define WORLDDATA_HPP

#include <string>
#include <vector>
#include <map>
#include "json.hpp" // <-- Changed to look in the current folder!

struct TeamInfo {
    std::string name;
    int level;           // 1-20
};

struct LeagueInfo {
    std::string name;
    int level;           // 1=top division, 2=second, etc.
    int roundsPerOpponent;
    std::vector<TeamInfo> teams;
};

struct CountryData {
    std::string country;
    std::vector<LeagueInfo> leagues;
};

class WorldData {
public:
    bool loadFromJson(const std::string& filepath);
    const std::map<std::string, CountryData>& getAllCountries() const;
    const CountryData* getCountry(const std::string& name) const;
    std::vector<TeamInfo> getAllTeams() const;

private:
    std::map<std::string, CountryData> countries;
};

#endif
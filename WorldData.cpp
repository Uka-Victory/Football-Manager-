#include "WorldData.hpp"
#include <fstream>
#include <iostream>

bool WorldData::loadFromJson(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
        return false;
    }
    nlohmann::json data;
    file >> data;
    
    for (auto& [countryName, countryJson] : data.items()) {
        CountryData cd;
        cd.country = countryName;
        if (countryJson.contains("leagues") && countryJson["leagues"].is_array()) {
            for (auto& leagueJson : countryJson["leagues"]) {
                LeagueInfo li;
                li.name = leagueJson.value("name", "Unknown League");
                li.level = leagueJson.value("level", 1);
                li.roundsPerOpponent = leagueJson.value("rounds_per_opponent", 2);
                if (leagueJson.contains("teams") && leagueJson["teams"].is_array()) {
                    for (auto& teamJson : leagueJson["teams"]) {
                        TeamInfo ti;
                        ti.name = teamJson.value("name", "Unknown Team");
                        ti.level = teamJson.value("level", 10);  // default 10 if missing
                        li.teams.push_back(ti);
                    }
                }
                cd.leagues.push_back(li);
            }
        }
        countries[countryName] = cd;
    }
    return true;
}

const std::map<std::string, CountryData>& WorldData::getAllCountries() const {
    return countries;
}

const CountryData* WorldData::getCountry(const std::string& name) const {
    auto it = countries.find(name);
    if (it != countries.end()) return &it->second;
    return nullptr;
}

std::vector<TeamInfo> WorldData::getAllTeams() const {
    std::vector<TeamInfo> all;
    for (const auto& [_, country] : countries) {
        for (const auto& league : country.leagues) {
            for (const auto& team : league.teams) {
                all.push_back(team);
            }
        }
    }
    return all;
}
#include "WorldData.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool WorldData::loadBaseDataFromJson(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filepath << " for base data." << std::endl;
        return false;
    }

    try {
        json j;
        file >> j;

        for (const auto& countryJson : j["countries"]) {
            CountryData cd;
            cd.country = countryJson.value("name", "Unknown");

            for (const auto& leagueJson : countryJson["leagues"]) {
                LeagueInfo li;
                li.name = leagueJson.value("name", "Unknown League");
                li.level = leagueJson.value("level", 1);
                li.roundsPerOpponent = leagueJson.value("roundsPerOpponent", 2);

                for (const auto& teamJson : leagueJson["teams"]) {
                    TeamInfo ti;
                    ti.name = teamJson.value("name", "Unknown Team");
                    ti.level = teamJson.value("level", li.level);
                    li.teams.push_back(ti);
                }
                cd.leagues.push_back(li);
            }
            baseCountries[cd.country] = cd;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "JSON Parsing Error in base data: " << e.what() << std::endl;
        return false;
    }
}

const std::unordered_map<std::string, CountryData>& WorldData::getBaseCountries() const {
    return baseCountries;
}

// --- DYNAMIC STATE MANAGEMENT ---

void WorldData::addTeamToWorld(TeamPtr team) {
    if (team) {
        globalTeamRegistry[team->getName()] = team; // Add to O(1) registry
    }
}

TeamPtr WorldData::getTeam(const std::string& teamName) const {
    auto it = globalTeamRegistry.find(teamName);
    if (it != globalTeamRegistry.end()) {
        return it->second;
    }
    return nullptr;
}

const std::unordered_map<std::string, TeamPtr>& WorldData::getGlobalTeamRegistry() const {
    return globalTeamRegistry;
}

void WorldData::addLeagueToWorld(LeaguePtr league) {
    if (league) {
        activeLeagues.push_back(league);
    }
}

const std::vector<LeaguePtr>& WorldData::getActiveLeagues() const {
    return activeLeagues;
}

LeaguePtr WorldData::getLeague(const std::string& leagueName) const {
    for (const auto& l : activeLeagues) {
        if (l->getName() == leagueName) {
            return l;
        }
    }
    return nullptr;
}

// --- SAVE / LOAD CAREER SYSTEM ---

bool WorldData::saveCareer(const std::string& saveFile) const {
    json j;

    // 1. Save all teams (this naturally triggers Team::toJson, saving all players inside them)
    json teamsJson = json::array();
    for (const auto& [name, teamPtr] : globalTeamRegistry) {
        teamsJson.push_back(teamPtr->toJson());
    }
    j["teams"] = teamsJson;

    // 2. Save all active leagues (they will only save team names and season standings)
    json leaguesJson = json::array();
    for (const auto& leaguePtr : activeLeagues) {
        leaguesJson.push_back(leaguePtr->toJson());
    }
    j["leagues"] = leaguesJson;

    // Write to file with a 4-space indent so it is human-readable
    std::ofstream file(saveFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << saveFile << " for saving." << std::endl;
        return false;
    }
    
    file << j.dump(4); 
    return true;
}

bool WorldData::loadCareer(const std::string& saveFile) {
    std::ifstream file(saveFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not find save file " << saveFile << std::endl;
        return false;
    }

    try {
        json j;
        file >> j;

        // Clear current active state before loading
        globalTeamRegistry.clear();
        activeLeagues.clear();

        // 1. Load Teams first, so the registry is populated
        if (j.contains("teams")) {
            for (const auto& teamJson : j["teams"]) {
                auto team = std::make_shared<Team>();
                team->fromJson(teamJson);
                addTeamToWorld(team);
            }
        }

        // 2. Load Leagues and pass them the populated registry to re-link pointers
        if (j.contains("leagues")) {
            for (const auto& leagueJson : j["leagues"]) {
                auto league = std::make_shared<League>();
                league->fromJson(leagueJson, globalTeamRegistry);
                addLeagueToWorld(league);
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "JSON Parsing Error while loading save: " << e.what() << std::endl;
        return false;
    }
}
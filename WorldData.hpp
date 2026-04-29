#ifndef WORLDDATA_HPP
#define WORLDDATA_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "json.hpp"
#include "League.hpp"
#include "Team.hpp"

// Structs for reading the initial setup data (world_data.json)
struct TeamInfo {
    std::string name;
    int level;
};

struct LeagueInfo {
    std::string name;
    int level;
    int roundsPerOpponent;
    std::vector<TeamInfo> teams;
};

struct CountryData {
    std::string country;
    std::vector<LeagueInfo> leagues;
};

class WorldData {
private:
    // Static data used just to generate a new game
    std::unordered_map<std::string, CountryData> baseCountries;

    // --- DYNAMIC GAME STATE (The actual living world) ---
    // This registry owns every team in the game. Leagues just point to this list.
    std::unordered_map<std::string, TeamPtr> globalTeamRegistry; 
    
    // The active leagues currently running in the career
    std::vector<LeaguePtr> activeLeagues;

public:
    WorldData() = default;

    // Base Data Loading (Used only when starting a New Game)
    bool loadBaseDataFromJson(const std::string& filepath);
    const std::unordered_map<std::string, CountryData>& getBaseCountries() const;

    // Dynamic State Management
    void addTeamToWorld(TeamPtr team);
    TeamPtr getTeam(const std::string& teamName) const;
    const std::unordered_map<std::string, TeamPtr>& getGlobalTeamRegistry() const;

    void addLeagueToWorld(LeaguePtr league);
    const std::vector<LeaguePtr>& getActiveLeagues() const;
    LeaguePtr getLeague(const std::string& leagueName) const;

    // --- SAVE / LOAD CAREER SYSTEM ---
    bool saveCareer(const std::string& saveFile) const;
    bool loadCareer(const std::string& saveFile);
};

#endif
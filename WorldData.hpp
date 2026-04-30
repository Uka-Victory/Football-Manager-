#ifndef WORLDDATA_HPP
#define WORLDDATA_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "json.hpp"
#include "League.hpp"
#include "Team.hpp"
#include "GameCalendar.hpp" // Added for time-saving

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
    std::unordered_map<std::string, CountryData> baseCountries;
    std::unordered_map<std::string, TeamPtr> globalTeamRegistry; 
    std::vector<LeaguePtr> activeLeagues;

public:
    WorldData() = default;

    bool loadBaseDataFromJson(const std::string& filepath);
    const std::unordered_map<std::string, CountryData>& getBaseCountries() const;

    void addTeamToWorld(TeamPtr team);
    TeamPtr getTeam(const std::string& teamName) const;
    const std::unordered_map<std::string, TeamPtr>& getGlobalTeamRegistry() const;

    void addLeagueToWorld(LeaguePtr league);
    const std::vector<LeaguePtr>& getActiveLeagues() const;
    LeaguePtr getLeague(const std::string& leagueName) const;

    // --- UPGRADED SAVE / LOAD SYSTEM ---
    // Now captures time and manager identity
    bool saveCareer(const std::string& saveFile, const GameCalendar& calendar, TeamPtr playerTeam, LeaguePtr playerLeague) const;
    bool loadCareer(const std::string& saveFile, GameCalendar& calendar, TeamPtr& playerTeam, LeaguePtr& playerLeague);
};

#endif
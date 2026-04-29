#ifndef LEAGUE_HPP
#define LEAGUE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Team.hpp"
#include "json.hpp"

// Tracks a team's performance for the current season
struct LeagueRecord {
    int played = 0;
    int won = 0;
    int drawn = 0;
    int lost = 0;
    int goalsFor = 0;
    int goalsAgainst = 0;
    int points = 0;

    int goalDifference() const { return goalsFor - goalsAgainst; }
};

class League {
private:
    std::string name;
    int level; // 1 = Top Flight (e.g. Premier League), 2 = Second Tier, etc.
    
    // Using Smart Pointers so the League just points to the global teams
    std::vector<TeamPtr> teams;
    
    // Maps a Team's Name to their current season record
    std::map<std::string, LeagueRecord> standings; 

public:
    League() = default;
    League(std::string leagueName, int tierLevel);

    // Getters
    std::string getName() const;
    int getLevel() const;

    // Team Management
    void addTeam(TeamPtr team);
    const std::vector<TeamPtr>& getTeams() const;
    TeamPtr getTeamByName(const std::string& teamName) const;

    // Season & Match Logic
    void recordMatch(const std::string& homeTeamName, const std::string& awayTeamName, int homeGoals, int awayGoals);
    void resetSeason(); // Crucial for Season Rollover
    
    // Returns the table sorted by Points, then Goal Difference, then Goals Scored
    std::vector<std::pair<TeamPtr, LeagueRecord>> getSortedStandings() const;

    // Save/Load System
    nlohmann::json toJson() const;
    // Note: We pass a global team registry so the league can reconnect its pointers to the right teams!
    void fromJson(const nlohmann::json& j, const std::unordered_map<std::string, TeamPtr>& globalTeamRegistry);
};

// Smart pointer typedef for Leagues
using LeaguePtr = std::shared_ptr<League>;

#endif
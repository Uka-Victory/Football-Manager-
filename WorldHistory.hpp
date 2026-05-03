#ifndef WORLDHISTORY_HPP
#define WORLDHISTORY_HPP

#include <string>
#include <vector>
#include <map>
#include "json.hpp"
#include "Team.hpp"

struct SeasonAward {
    std::string category;
    std::string winnerPlayerId;
    std::string winnerName;
    std::string teamName;
    int value = 0; // The stat value (e.g., total goals, or avgRating * 100)
};

class WorldHistory {
public:
    // Map: Competition Name -> Year -> Winning Team Name
    std::map<std::string, std::map<int, std::string>> competitionWinners;
    // Map: Year -> Vector of generated awards
    std::map<int, std::vector<SeasonAward>> awards;

    void addCompetitionWinner(const std::string& compName, int year, const std::string& teamName);
    void addAward(int year, const SeasonAward& award);
    
    // Scans all teams in a league to determine statistical award winners
    void generateSeasonalAwards(int year, const std::string& leagueName, const std::vector<TeamPtr>& leagueTeams);

    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

#endif
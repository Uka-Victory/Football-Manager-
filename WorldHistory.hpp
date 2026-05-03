#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "json.hpp"
#include "Team.hpp"

namespace FootballManager {

    struct SeasonAward {
        std::string category;
        std::string winnerPlayerId;
        std::string winnerName;
        std::string teamName;
        int value = 0;
    };

    class WorldHistory {
    public:
        std::map<std::string, std::map<int, std::string>> competitionWinners;
        std::map<int, std::vector<SeasonAward>> awards;

        void addCompetitionWinner(const std::string& compName, int year, const std::string& teamName);
        void addAward(int year, const SeasonAward& award);

        // Fix: Use std::shared_ptr<Team> directly — TeamPtr was undefined outside the namespace
        void generateSeasonalAwards(int year, const std::string& leagueName,
                                    const std::vector<std::shared_ptr<Team>>& leagueTeams);

        nlohmann::json toJson() const;
        void fromJson(const nlohmann::json& j);
    };

} // namespace FootballManager
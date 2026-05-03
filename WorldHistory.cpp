#include "WorldHistory.hpp"
#include <algorithm>

void WorldHistory::addCompetitionWinner(const std::string& compName, int year, const std::string& teamName) {
    competitionWinners[compName][year] = teamName;
}

void WorldHistory::addAward(int year, const SeasonAward& award) {
    awards[year].push_back(award);
}

void WorldHistory::generateSeasonalAwards(int year, const std::string& leagueName, const std::vector<TeamPtr>& leagueTeams) {
    if (leagueTeams.empty()) return;

    PlayerPtr topScorer = nullptr;
    PlayerPtr bestPlayer = nullptr;
    std::string tsTeam = "";
    std::string bpTeam = "";

    for (const auto& team : leagueTeams) {
        for (const auto& player : team->getSeniorSquad()) {
            // Find Top Scorer
            if (!topScorer || player->goals > topScorer->goals) {
                topScorer = player;
                tsTeam = team->getName();
            }
            
            // Find Player of the Year (Requires at least 10 appearances to qualify)
            if (player->appearances >= 10) {
                if (!bestPlayer || player->avgRating > bestPlayer->avgRating) {
                    bestPlayer = player;
                    bpTeam = team->getName();
                }
            }
        }
    }

    if (topScorer && topScorer->goals > 0) {
        SeasonAward tsAward{leagueName + " Golden Boot", topScorer->uniqueId, topScorer->name, tsTeam, topScorer->goals};
        addAward(year, tsAward);
    }

    if (bestPlayer) {
        SeasonAward bpAward{leagueName + " Player of the Year", bestPlayer->uniqueId, bestPlayer->name, bpTeam, static_cast<int>(bestPlayer->avgRating * 100)};
        addAward(year, bpAward);
    }
}

nlohmann::json WorldHistory::toJson() const {
    nlohmann::json j;
    
    nlohmann::json compJson = nlohmann::json::object();
    for (const auto& [compName, yearMap] : competitionWinners) {
        nlohmann::json yearJson = nlohmann::json::object();
        for (const auto& [year, team] : yearMap) {
            yearJson[std::to_string(year)] = team;
        }
        compJson[compName] = yearJson;
    }
    j["competitionWinners"] = compJson;

    nlohmann::json awardsJson = nlohmann::json::array();
    for (const auto& [year, awardVec] : awards) {
        for (const auto& award : awardVec) {
            awardsJson.push_back({
                {"year", year},
                {"category", award.category},
                {"winnerPlayerId", award.winnerPlayerId},
                {"winnerName", award.winnerName},
                {"teamName", award.teamName},
                {"value", award.value}
            });
        }
    }
    j["awards"] = awardsJson;

    return j;
}

void WorldHistory::fromJson(const nlohmann::json& j) {
    competitionWinners.clear();
    if (j.contains("competitionWinners")) {
        for (const auto& [compName, yearObj] : j["competitionWinners"].items()) {
            std::map<int, std::string> yearMap;
            for (const auto& [yearStr, team] : yearObj.items()) {
                yearMap[std::stoi(yearStr)] = team.get<std::string>();
            }
            competitionWinners[compName] = yearMap;
        }
    }

    awards.clear();
    if (j.contains("awards")) {
        for (const auto& a : j["awards"]) {
            SeasonAward award;
            int year = a.value("year", 2024);
            award.category = a.value("category", "");
            award.winnerPlayerId = a.value("winnerPlayerId", "");
            award.winnerName = a.value("winnerName", "");
            award.teamName = a.value("teamName", "");
            award.value = a.value("value", 0);
            awards[year].push_back(award);
        }
    }
}
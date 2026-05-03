WorldHistory.cpp
#include "WorldHistory.hpp"
#include <algorithm>

namespace FootballManager {

    void WorldHistory::addCompetitionWinner(const std::string& compName, int year, const std::string& teamName) {
        competitionWinners[compName][year] = teamName;
    }

    void WorldHistory::addAward(int year, const SeasonAward& award) {
        awards[year].push_back(award);
    }

    void WorldHistory::generateSeasonalAwards(int year, const std::string& leagueName,
                                               const std::vector<std::shared_ptr<Team>>& leagueTeams) {
        if (leagueTeams.empty()) return;

        std::shared_ptr<Player> topScorer = nullptr;
        std::shared_ptr<Player> bestPlayer = nullptr;
        std::string tsTeam, bpTeam;

        for (const auto& team : leagueTeams) {
            for (const auto& player : team->getSeniorSquad()) {

                // Fix: Use getStats() getters instead of direct private field access
                if (!topScorer || player->getStats().goals > topScorer->getStats().goals) {
                    topScorer = player;
                    tsTeam = team->getName();
                }

                if (player->getStats().appearances >= 10) {
                    if (!bestPlayer || player->getStats().averageRating > bestPlayer->getStats().averageRating) {
                        bestPlayer = player;
                        bpTeam = team->getName();
                    }
                }
            }
        }

        // Fix: Use getId() and getName() instead of ->uniqueId and ->name
        if (topScorer && topScorer->getStats().goals > 0) {
            SeasonAward tsAward{
                leagueName + " Golden Boot",
                topScorer->getId(),
                topScorer->getName(),
                tsTeam,
                topScorer->getStats().goals
            };
            addAward(year, tsAward);
        }

        if (bestPlayer) {
            SeasonAward bpAward{
                leagueName + " Player of the Year",
                bestPlayer->getId(),
                bestPlayer->getName(),
                bpTeam,
                static_cast<int>(bestPlayer->getStats().averageRating * 100)
            };
            addAward(year, bpAward);
        }
    }

    nlohmann::json WorldHistory::toJson() const {
        nlohmann::json j;

        nlohmann::json compJson = nlohmann::json::object();
        for (const auto& [compName, yearMap] : competitionWinners) {
            nlohmann::json yearJson = nlohmann::json::object();
            for (const auto& [yr, team] : yearMap) {
                yearJson[std::to_string(yr)] = team;
            }
            compJson[compName] = yearJson;
        }
        j["competitionWinners"] = compJson;

        nlohmann::json awardsJson = nlohmann::json::array();
        for (const auto& [yr, awardVec] : awards) {
            for (const auto& award : awardVec) {
                awardsJson.push_back({
                    {"year",           yr},
                    {"category",       award.category},
                    {"winnerPlayerId", award.winnerPlayerId},
                    {"winnerName",     award.winnerName},
                    {"teamName",       award.teamName},
                    {"value",          award.value}
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
                int yr           = a.value("year", 2024);
                award.category       = a.value("category", "");
                award.winnerPlayerId = a.value("winnerPlayerId", "");
                award.winnerName     = a.value("winnerName", "");
                award.teamName       = a.value("teamName", "");
                award.value          = a.value("value", 0);
                awards[yr].push_back(award);
            }
        }
    }

} // namespace FootballManager
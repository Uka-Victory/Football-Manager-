// WorldHistory.cpp
#include "WorldHistory.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

// ========== COMPETITION WINNERS ==========
void WorldHistory::addCompetitionWinner(const std::string& competition, int year,
                                        const std::string& winner, const std::string& runnerUp) {
    competitionWinners[competition][year] = winner;
    if (!runnerUp.empty())
        competitionRunnersUp[competition][year] = runnerUp;
}

std::string WorldHistory::getCompetitionWinner(const std::string& competition, int year) const {
    auto it = competitionWinners.find(competition);
    if (it != competitionWinners.end()) {
        auto yit = it->second.find(year);
        if (yit != it->second.end()) return yit->second;
    }
    return "";
}

std::vector<std::pair<int, std::string>> WorldHistory::getCompetitionHistory(
    const std::string& competition) const {
    std::vector<std::pair<int, std::string>> history;
    auto it = competitionWinners.find(competition);
    if (it != competitionWinners.end()) {
        for (const auto& kv : it->second)
            history.push_back({kv.first, kv.second});
    }
    std::sort(history.begin(), history.end());
    return history;
}

// ========== AWARDS ==========
void WorldHistory::computeSeasonAwards(
    const std::vector<std::shared_ptr<League>>& leagues,
    const std::map<std::string, PlayerPtr>& players) {
    
    int season = 0;
    // Determine season year from the first league (assume all are same)
    for (const auto& league : leagues) {
        if (!league->getTeams().empty()) {
            season = league->getSeasonYear();
            break;
        }
    }
    if (season == 0) return;

    // Per league: Golden Boot (most goals), Golden Glove (most clean sheets, GK),
    // Player of the Year (highest avg rating, min 20 apps)
    for (const auto& league : leagues) {
        const auto& teams = league->getTeams();
        if (teams.empty()) continue;

        // Gather all players from this league
        std::vector<PlayerPtr> leaguePlayers;
        for (const auto& team : teams) {
            for (const auto& p : team->getSeniorSquad())
                leaguePlayers.push_back(p);
            for (const auto& p : team->getYouthSquad())
                leaguePlayers.push_back(p);
        }

        // Golden Boot
        PlayerPtr topScorer = nullptr;
        int maxGoals = 0;
        for (const auto& p : leaguePlayers) {
            if (p->getGoals() > maxGoals) {
                maxGoals = p->getGoals();
                topScorer = p;
            }
        }
        if (topScorer && maxGoals > 0) {
            SeasonAward gb;
            gb.category = "Golden Boot";
            gb.seasonYear = season;
            gb.winnerPlayerId = topScorer->getUniqueId();
            gb.winnerName = topScorer->getName();
            // Find team
            for (const auto& team : teams) {
                auto tp = team->getPlayerById(topScorer->getUniqueId());
                if (tp) { gb.teamName = team->getName(); break; }
            }
            gb.value = maxGoals;
            addAward(gb);
        }

        // Golden Glove (GK with most clean sheets, min 15 apps)
        PlayerPtr topGK = nullptr;
        int maxClean = 0;
        for (const auto& p : leaguePlayers) {
            if (p->getPrimaryPosition() == "GK" && p->getAppearances() >= 15) {
                if (p->getCleanSheets() > maxClean) {
                    maxClean = p->getCleanSheets();
                    topGK = p;
                }
            }
        }
        if (topGK && maxClean > 0) {
            SeasonAward gg;
            gg.category = "Golden Glove";
            gg.seasonYear = season;
            gg.winnerPlayerId = topGK->getUniqueId();
            gg.winnerName = topGK->getName();
            for (const auto& team : teams) {
                auto tp = team->getPlayerById(topGK->getUniqueId());
                if (tp) { gg.teamName = team->getName(); break; }
            }
            gg.value = maxClean;
            addAward(gg);
        }

        // Player of the Year (highest avg rating, min 20 apps)
        PlayerPtr poty = nullptr;
        double bestRating = 0.0;
        for (const auto& p : leaguePlayers) {
            if (p->getAppearances() >= 20) {
                if (p->getAvgRating() > bestRating) {
                    bestRating = p->getAvgRating();
                    poty = p;
                }
            }
        }
        if (poty) {
            SeasonAward py;
            py.category = "Player of the Year";
            py.seasonYear = season;
            py.winnerPlayerId = poty->getUniqueId();
            py.winnerName = poty->getName();
            for (const auto& team : teams) {
                auto tp = team->getPlayerById(poty->getUniqueId());
                if (tp) { py.teamName = team->getName(); break; }
            }
            py.value = static_cast<int>(bestRating * 10);
            addAward(py);
        }

        // Update global records for goals
        if (maxGoals > mostGoalsSeason) {
            mostGoalsSeason = maxGoals;
            mostGoalsPlayerId = topScorer ? topScorer->getUniqueId() : "";
        }
        if (maxClean > mostCleanSheetsSeason) {
            mostCleanSheetsSeason = maxClean;
            mostCleanSheetsPlayerId = topGK ? topGK->getUniqueId() : "";
        }
    }
}

void WorldHistory::addAward(const SeasonAward& award) {
    seasonAwards[award.seasonYear].push_back(award);
}

std::vector<SeasonAward> WorldHistory::getAwardsForSeason(int year) const {
    auto it = seasonAwards.find(year);
    if (it != seasonAwards.end()) return it->second;
    return {};
}

// ========== HALL OF FAME ==========
void WorldHistory::inductPlayerIntoHallOfFame(const std::string& playerId) {
    if (std::find(hallOfFamePlayers.begin(), hallOfFamePlayers.end(), playerId) 
        == hallOfFamePlayers.end())
        hallOfFamePlayers.push_back(playerId);
}

void WorldHistory::inductManagerIntoHallOfFame(const std::string& managerId) {
    if (std::find(hallOfFameManagers.begin(), hallOfFameManagers.end(), managerId)
        == hallOfFameManagers.end())
        hallOfFameManagers.push_back(managerId);
}

const std::vector<std::string>& WorldHistory::getHallOfFamePlayers() const {
    return hallOfFamePlayers;
}

const std::vector<std::string>& WorldHistory::getHallOfFameManagers() const {
    return hallOfFameManagers;
}

// ========== GLOBAL RECORDS ==========
void WorldHistory::updateHighestTransfer(int64_t fee) {
    if (fee > highestTransferEver) highestTransferEver = fee;
}

void WorldHistory::updateMostGoalsSeason(int goals, const std::string& playerId) {
    if (goals > mostGoalsSeason) {
        mostGoalsSeason = goals;
        mostGoalsPlayerId = playerId;
    }
}

int64_t WorldHistory::getHighestTransferEver() const {
    return highestTransferEver;
}

int WorldHistory::getMostGoalsSeason() const {
    return mostGoalsSeason;
}

// ========== COMPETITION LEADERBOARDS ==========
void WorldHistory::updateCompetitionLeaderboards(const std::string& competition,
                                                 const PlayerPtr& player,
                                                 int goals, int assists) {
    if (!player) return;
    const std::string& pid = player->getUniqueId();
    const std::string& pname = player->getName();

    // Update goal leaderboard
    auto& goalList = topScorersByCompetition[competition];
    bool found = false;
    for (auto& entry : goalList) {
        if (entry.playerId == pid) {
            entry.total += goals;
            found = true;
            break;
        }
    }
    if (!found && goals > 0) {
        goalList.push_back({pid, pname, goals});
    }
    // Sort and keep top 100
    std::sort(goalList.begin(), goalList.end(),
        [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            return a.total > b.total;
        });
    if (goalList.size() > 100) goalList.resize(100);

    // Update assist leaderboard
    auto& assistList = topAssistersByCompetition[competition];
    found = false;
    for (auto& entry : assistList) {
        if (entry.playerId == pid) {
            entry.total += assists;
            found = true;
            break;
        }
    }
    if (!found && assists > 0) {
        assistList.push_back({pid, pname, assists});
    }
    std::sort(assistList.begin(), assistList.end(),
        [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            return a.total > b.total;
        });
    if (assistList.size() > 100) assistList.resize(100);
}

std::vector<WorldHistory::LeaderboardEntry> WorldHistory::getTopScorers(
    const std::string& competition, int limit) const {
    auto it = topScorersByCompetition.find(competition);
    if (it != topScorersByCompetition.end()) {
        auto list = it->second;
        if ((int)list.size() > limit) list.resize(limit);
        return list;
    }
    return {};
}

std::vector<WorldHistory::LeaderboardEntry> WorldHistory::getTopAssisters(
    const std::string& competition, int limit) const {
    auto it = topAssistersByCompetition.find(competition);
    if (it != topAssistersByCompetition.end()) {
        auto list = it->second;
        if ((int)list.size() > limit) list.resize(limit);
        return list;
    }
    return {};
}

// ========== PERSISTENCE ==========
json WorldHistory::toJson() const {
    json j;

    // Competition winners
    json cw = json::object();
    for (const auto& [comp, yearMap] : competitionWinners) {
        json ym = json::object();
        for (const auto& [year, team] : yearMap) ym[std::to_string(year)] = team;
        cw[comp] = ym;
    }
    j["competitionWinners"] = cw;

    // Awards
    json awardsJson = json::array();
    for (const auto& [year, awardVec] : seasonAwards) {
        for (const auto& award : awardVec) {
            awardsJson.push_back({
                {"category", award.category},
                {"winnerPlayerId", award.winnerPlayerId},
                {"winnerName", award.winnerName},
                {"teamName", award.teamName},
                {"value", award.value},
                {"seasonYear", award.seasonYear}
            });
        }
    }
    j["awards"] = awardsJson;

    // Hall of Fame
    j["hallOfFamePlayers"] = hallOfFamePlayers;
    j["hallOfFameManagers"] = hallOfFameManagers;

    // Records
    j["highestTransferEver"] = highestTransferEver;
    j["mostGoalsSeason"] = mostGoalsSeason;
    j["mostGoalsPlayerId"] = mostGoalsPlayerId;

    // Leaderboards
    auto lbToJson = [](const std::map<std::string, std::vector<LeaderboardEntry>>& lb) {
        json out = json::object();
        for (const auto& [comp, entries] : lb) {
            json arr = json::array();
            for (const auto& e : entries)
                arr.push_back({{"playerId", e.playerId}, {"playerName", e.playerName}, {"total", e.total}});
            out[comp] = arr;
        }
        return out;
    };
    j["topScorersByCompetition"] = lbToJson(topScorersByCompetition);
    j["topAssistersByCompetition"] = lbToJson(topAssistersByCompetition);

    return j;
}

void WorldHistory::fromJson(const json& j) {
    // Winners
    if (j.contains("competitionWinners")) {
        for (const auto& [comp, yearObj] : j["competitionWinners"].items()) {
            for (const auto& [yearStr, team] : yearObj.items()) {
                competitionWinners[comp][std::stoi(yearStr)] = team.get<std::string>();
            }
        }
    }

    // Awards
    if (j.contains("awards")) {
        for (const auto& aj : j["awards"]) {
            SeasonAward a;
            a.category = aj.value("category", "");
            a.winnerPlayerId = aj.value("winnerPlayerId", "");
            a.winnerName = aj.value("winnerName", "");
            a.teamName = aj.value("teamName", "");
            a.value = aj.value("value", 0);
            a.seasonYear = aj.value("seasonYear", 0);
            seasonAwards[a.seasonYear].push_back(a);
        }
    }

    // Hall of Fame
    if (j.contains("hallOfFamePlayers"))
        hallOfFamePlayers = j["hallOfFamePlayers"].get<std::vector<std::string>>();
    if (j.contains("hallOfFameManagers"))
        hallOfFameManagers = j["hallOfFameManagers"].get<std::vector<std::string>>();

    // Records
    highestTransferEver = j.value("highestTransferEver", 0);
    mostGoalsSeason = j.value("mostGoalsSeason", 0);
    mostGoalsPlayerId = j.value("mostGoalsPlayerId", "");

    // Leaderboards
    auto parseLb = [](const json& src, std::map<std::string, std::vector<LeaderboardEntry>>& dest) {
        if (src.is_null()) return;
        for (const auto& [comp, arr] : src.items()) {
            std::vector<LeaderboardEntry> entries;
            for (const auto& e : arr) {
                entries.push_back({e.value("playerId", ""), e.value("playerName", ""), e.value("total", 0)});
            }
            dest[comp] = entries;
        }
    };
    if (j.contains("topScorersByCompetition"))
        parseLb(j["topScorersByCompetition"], topScorersByCompetition);
    if (j.contains("topAssistersByCompetition"))
        parseLb(j["topAssistersByCompetition"], topAssistersByCompetition);
}

void WorldHistory::saveToFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (out) out << toJson().dump(2);
}

void WorldHistory::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (in) {
        json j;
        in >> j;
        fromJson(j);
    }
}
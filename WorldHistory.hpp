// WorldHistory.hpp
#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Player.hpp"
#include "Team.hpp"
#include "League.hpp"
#include "json.hpp"
using json = nlohmann::json;

struct SeasonAward {
    std::string category;
    std::string winnerPlayerId;
    std::string winnerName;
    std::string teamName;
    int value = 0;
    int seasonYear = 0;
};

class WorldHistory {
private:
    // competitionName -> (year -> winnerTeamName)
    std::map<std::string, std::map<int, std::string>> competitionWinners;
    // Runner-ups
    std::map<std::string, std::map<int, std::string>> competitionRunnersUp;
    // Year -> list of awards
    std::map<int, std::vector<SeasonAward>> seasonAwards;
    // Hall of Fame (player IDs)
    std::vector<std::string> hallOfFamePlayers;
    std::vector<std::string> hallOfFameManagers;
    // Global records
    int64_t highestTransferEver = 0;
    int mostGoalsSeason = 0;
    int mostCleanSheetsSeason = 0;
    std::string mostGoalsPlayerId;
    std::string mostCleanSheetsPlayerId;
    // Per-competition leaderboards
    struct LeaderboardEntry {
        std::string playerId;
        std::string playerName;
        int total = 0;
    };
    std::map<std::string, std::vector<LeaderboardEntry>> topScorersByCompetition;
    std::map<std::string, std::vector<LeaderboardEntry>> topAssistersByCompetition;

public:
    // ========== COMPETITION WINNERS ==========
    void addCompetitionWinner(const std::string& competition, int year, 
                              const std::string& winner, const std::string& runnerUp = "");
    std::string getCompetitionWinner(const std::string& competition, int year) const;
    std::vector<std::pair<int, std::string>> getCompetitionHistory(const std::string& competition) const;

    // ========== AWARDS ==========
    void computeSeasonAwards(const std::vector<std::shared_ptr<League>>& leagues,
                             const std::map<std::string, PlayerPtr>& players);
    void addAward(const SeasonAward& award);
    std::vector<SeasonAward> getAwardsForSeason(int year) const;

    // ========== HALL OF FAME ==========
    void inductPlayerIntoHallOfFame(const std::string& playerId);
    void inductManagerIntoHallOfFame(const std::string& managerId);
    const std::vector<std::string>& getHallOfFamePlayers() const;
    const std::vector<std::string>& getHallOfFameManagers() const;

    // ========== GLOBAL RECORDS ==========
    void updateHighestTransfer(int64_t fee);
    void updateMostGoalsSeason(int goals, const std::string& playerId);
    int64_t getHighestTransferEver() const;
    int getMostGoalsSeason() const;

    // ========== COMPETITION LEADERBOARDS ==========
    void updateCompetitionLeaderboards(const std::string& competition,
                                       const PlayerPtr& player, int goals, int assists);
    std::vector<LeaderboardEntry> getTopScorers(const std::string& competition, int limit = 10) const;
    std::vector<LeaderboardEntry> getTopAssisters(const std::string& competition, int limit = 10) const;

    // ========== PERSISTENCE ==========
    json toJson() const;
    void fromJson(const json& j);
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    using WorldHistoryPtr = std::shared_ptr<WorldHistory>;
};
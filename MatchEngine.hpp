// MatchEngine.hpp
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Team.hpp"
#include "Player.hpp"

// ========== MATCH EVENT ==========
struct MatchEvent {
    enum Type { GOAL, YELLOW_CARD, RED_CARD, ASSIST };
    Type type;
    int minute;
    std::string playerId;
    std::string assistPlayerId;   // for goals
};

// ========== PER‑PLAYER MATCH STATS ==========
struct PlayerMatchStats {
    int minutesPlayed = 0;
    int goals = 0;
    int assists = 0;
    double xg = 0.0;
    double xa = 0.0;
    double xt = 0.0;
    double xgChain = 0.0;
    double xgBuildup = 0.0;

    int shots = 0;
    int shotsOnTarget = 0;
    int bigChancesMissed = 0;

    int passesAttempted = 0;
    int passesCompleted = 0;
    int progressivePasses = 0;
    int passesIntoFinalThird = 0;
    int passesIntoPenaltyArea = 0;
    int throughBalls = 0;
    int crossesAttempted = 0;
    int crossesCompleted = 0;
    int switchesOfPlay = 0;

    int dribblesAttempted = 0;
    int dribblesCompleted = 0;
    int progressiveCarries = 0;

    int chancesCreated = 0;      // key passes
    int bigChancesCreated = 0;
    int sca = 0;               // shot‑creating actions
    int gca = 0;               // goal‑creating actions

    int tacklesAttempted = 0;
    int tacklesWon = 0;
    int interceptions = 0;
    int blocks = 0;
    int clearances = 0;
    int headedClearances = 0;
    int recoveries = 0;

    int groundDuelsAttempted = 0;
    int groundDuelsWon = 0;
    int aerialDuelsAttempted = 0;
    int aerialDuelsWon = 0;

    int foulsCommitted = 0;
    int foulsDrawn = 0;
    int yellowCards = 0;
    int redCards = 0;
    int errorsLeadingToShot = 0;
    int errorsLeadingToGoal = 0;

    int saves = 0;
    int goalsConceded = 0;
    int cleanSheet = 0;
    int highClaims = 0;
    double distributionAccuracy = 0.0;

    double distanceCovered = 0.0;
    double sprintDistance = 0.0;
    double topSpeed = 0.0;

    double matchRating = 6.0;
};

// ========== MATCH RESULT ==========
struct MatchResult {
    int homeGoals = 0;
    int awayGoals = 0;
    std::vector<MatchEvent> events;
    std::unordered_map<std::string, PlayerMatchStats> playerStats;
    std::unordered_map<std::string, int> playerGoals;
    std::unordered_map<std::string, int> playerAssists;
};

// ========== MATCH ENGINE ==========
class MatchEngine {
public:
    static MatchResult simulateMatch(const TeamPtr& home, const TeamPtr& away);
    static void applyResultToTeams(const TeamPtr& home, const TeamPtr& away,
                                   MatchResult& res, const std::string& date);

private:
    static double computeAttackStrength(const std::vector<PlayerPtr>& xi);
    static double computeDefenceStrength(const std::vector<PlayerPtr>& xi);
    static double computeMidfieldControl(const std::vector<PlayerPtr>& xi);
    static double zoneThreat(int col, int row);
    static double shotXg(int col, int row, double pressure);
    static void simulatePossession(const std::vector<PlayerPtr>& attackXI,
                                   const std::vector<PlayerPtr>& defendXI,
                                   MatchResult& res,
                                   bool isHome,
                                   double homeControl, double awayControl);
    static double calculateMatchRating(const PlayerMatchStats& stats,
                                       const std::string& position);
};
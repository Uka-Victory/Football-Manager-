// MatchStats.hpp
// Shared match statistics structs — extracted to break circular dependency
// between Team.hpp and MatchEngine.hpp
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// ========== MATCH EVENT ==========
struct MatchEvent {
    enum Type { GOAL, YELLOW_CARD, RED_CARD, ASSIST };
    Type type;
    int minute = 0;
    std::string playerId;
    std::string assistPlayerId;
};

// ========== PER-PLAYER MATCH STATS ==========
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

    int chancesCreated = 0;
    int bigChancesCreated = 0;
    int sca = 0;
    int gca = 0;

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
    int dispossessed = 0;
    int dribbledPast = 0;
    int offsides = 0;
    int penaltiesMissed = 0;
    int ownGoals = 0;

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

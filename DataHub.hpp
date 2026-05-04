// DataHub.hpp
#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Player.hpp"
#include "Team.hpp"
#include "League.hpp"
#include "MatchEngine.hpp"

// ========== OPPONENT REPORT ==========
struct OpponentStyleProfile {
    double possessionAvg = 50.0;
    double passDirectness = 50.0;       // % short vs long
    double pressingIntensity = 50.0;
    double crossingFrequency = 5.0;     // per match
    double avgShotDistance = 17.0;      // yards
    double defensiveLineHeight = 40.0;  // avg column
};

struct OpponentKeyPlayer {
    std::string playerId;
    std::string name;
    std::string role;          // "Top Scorer", "Playmaker", etc.
    double statValue = 0.0;
    std::string statLabel;     // "goals", "xT", etc.
};

struct OpponentWeakness {
    std::string description;   // e.g. "Left flank vulnerable"
    std::string positionGroup;
    double severity = 1.0;
};

struct OpponentReport {
    std::string opponentTeamName;
    OpponentStyleProfile style;
    std::vector<OpponentKeyPlayer> keyPlayers;
    std::vector<OpponentWeakness> weaknesses;
    int matchesAnalysed = 0;
};

// ========== OWN TEAM ANALYSIS ==========
struct SquadOverviewEntry {
    std::string playerId;
    std::string name;
    int minutesPlayed = 0;
    int goals = 0, assists = 0;
    double xg = 0.0, xa = 0.0, xt = 0.0;
    double avgRating = 0.0;
    int progressivePasses = 0, tacklesWon = 0, interceptions = 0;
    int cards = 0;
};

struct PositionalAnalysis {
    std::string positionGroup;
    int playerCount = 0;
    double avgRating = 0.0;
    bool weaknessFlag = false;
};

struct TacticalEffectiveness {
    int matchesPlayed = 0, wins = 0, draws = 0, losses = 0;
    int goalsScored = 0, goalsConceded = 0;
    int setPieceGoals = 0;
    double pressSuccess = 0.0;  // interceptions in opponent half per match
};

// ========== SCOUT REPORT ==========
struct ScoutReportCard {
    std::string playerId;
    std::string name;
    int estimatedOverall = 0;
    int estimatedPotential = 0;
    std::string playstyle;
    std::vector<std::string> knownTraits;
    std::map<std::string, std::pair<int,int>> attributeRanges; // min-max
    int64_t estimatedValue = 0;
    int estimatedWage = 0;
    int contractMonthsLeft = 0;
    double recentAvgRating = 0.0;
    std::string scoutVerdict;   // "Strongly Recommend" etc.
};

struct PlayerComparison {
    std::string ownPlayerName, scoutedPlayerName;
    std::map<std::string, std::pair<int,int>> attributeComparison; // own vs scout
    std::map<std::string, std::pair<double,double>> statComparison;
};

// ========== MATCH DATA STORAGE ==========
struct MatchDataEntry {
    std::string date;
    std::string competition;
    std::string homeTeam, awayTeam;
    int homeGoals, awayGoals;
    std::map<std::string, PlayerMatchStats> playerStats;   // all players involved
    std::vector<MatchEvent> events;
};

class DataHub {
private:
    std::string m_managedClubName;
    std::vector<MatchDataEntry> m_allMatches;               // current season only

    // Cached analysis (recomputed when data changes)
    std::map<std::string, OpponentReport> m_opponentCache;
    std::vector<SquadOverviewEntry> m_squadOverviewCache;
    std::vector<PositionalAnalysis> m_positionalCache;
    TacticalEffectiveness m_tacticalCache;
    std::vector<ScoutReportCard> m_scoutReports;

    bool m_cacheDirty = true;

    void recomputeCaches();

public:
    DataHub() = default;

    // ========== INGESTION ==========
    void ingestMatch(const Fixture& fix, const MatchResult& res);
    void setManagedClub(const std::string& name) { m_managedClubName = name; }

    // ========== OPPONENT ANALYSIS ==========
    OpponentReport getOpponentReport(const std::string& opponentTeamName);
    OpponentStyleProfile computeTeamStyle(const std::string& teamName) const;

    // ========== OWN TEAM ANALYSIS ==========
    std::vector<SquadOverviewEntry> getSquadOverview();
    std::vector<PositionalAnalysis> getPositionalAnalysis();
    TacticalEffectiveness getTacticalEffectiveness();

    // ========== SCOUT REPORTS ==========
    void addScoutReport(const ScoutReportCard& report);
    std::vector<ScoutReportCard> getScoutReports() const;
    PlayerComparison compareWithScouted(const std::string& scoutedPlayerId,
                                        const PlayerPtr& ownPlayer) const;

    // ========== SEASON LIFECYCLE ==========
    void resetSeason();
    json toJson() const;
    void fromJson(const json& j);
};
// Team.hpp
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "Player.hpp"
#include "Facilities.hpp"
#include "json.hpp"

using json = nlohmann::json;

// ========== DEPTH CHART ENTRY ==========
struct DepthChartEntry {
    std::string playerId;
    int rank = 1;   // 1 = first choice
};

// ========== TROPHY ENTRY ==========
struct TrophyEntry {
    std::string competitionName;
    int year;
    int tier = 0;   // 0 for non‑league competitions
};

// ========== CLUB STAT LEADER ==========
struct ClubStatLeader {
    std::string playerId;
    int total = 0;
};

// ========== HEAD‑TO‑HEAD STRUCTS ==========
struct PlayerH2HStats {
    std::string playerId;
    int appearances = 0;
    int goals = 0;
    int assists = 0;
    double totalRating = 0.0;   // sum of match ratings, for averaging
};

struct HeadToHeadRecord {
    int wins = 0, draws = 0, losses = 0;
    int goalsFor = 0, goalsAgainst = 0;
    int biggestWinMargin = 0;
    int biggestDefeatMargin = 0;
    std::string firstMeetingDate;
    std::string lastMeetingDate;

    std::map<std::string, PlayerH2HStats> playerStats;  // key = playerId

    // helper queries (const, to be called from UI later)
    std::vector<PlayerH2HStats> getTopScorers(int limit = 5) const;
    std::vector<PlayerH2HStats> getTopAssisters(int limit = 5) const;
    std::vector<PlayerH2HStats> getMostAppearances(int limit = 5) const;
    std::vector<PlayerH2HStats> getHighestAvgRating(int limit = 5, int minApps = 3) const;
};

// ========== TEAM / CLUB CLASS ==========
class Team {
private:
    // Identity
    std::string m_name;
    std::string m_country;
    int m_clubLevel = 10;                // 1‑20
    int m_reputation = 50;               // 1‑100
    std::string m_primaryColour;
    std::string m_secondaryColour;
    std::string m_stadiumName;
    int m_foundedYear = 1900;

    // Squad
    std::vector<PlayerPtr> m_seniorSquad;   // 18‑35 players
    std::vector<PlayerPtr> m_youthSquad;    // 16‑21
    std::vector<PlayerPtr> m_academy;       // 14‑15, one season only

    // Depth chart: position group → ordered list
    std::map<std::string, std::vector<DepthChartEntry>> m_depthChart;

    // Facilities
    Facilities m_facilities;

    // Season record
    int m_gamesPlayed = 0, m_wins = 0, m_draws = 0, m_losses = 0;
    int m_goalsFor = 0, m_goalsAgainst = 0, m_points = 0;

    // Tactics placeholder
    std::string m_formationName = "4-3-3";

    // Club records
    int m_biggestWinMargin = 0;
    int m_heaviestDefeatMargin = 0;
    int m_mostPointsSeason = 0;
    int m_mostGoalsSeason = 0;
    int64_t m_highestTransferPaid = 0;
    int64_t m_highestTransferReceived = 0;
    int m_youngestPlayerAge = 99;
    int m_oldestPlayerAge = 0;
    std::string m_mostAppearancesPlayerId;

    // Trophy cabinet
    std::vector<TrophyEntry> m_trophyCabinet;
    std::map<std::string, int> m_trophyCounts;   // competitionName → count

    // All‑time club top lists (top 20, recomputed seasonally)
    std::vector<ClubStatLeader> m_topScorers;
    std::vector<ClubStatLeader> m_topAssisters;
    std::vector<ClubStatLeader> m_topAppearances;

    // Club legends (player IDs)
    std::vector<std::string> m_clubLegends;

    // Head‑to‑head (opponent club name → record)
    std::map<std::string, HeadToHeadRecord> m_headToHead;

    // Rivals (club names)
    std::vector<std::string> m_rivalClubNames;

    // Board expectations
    std::string m_boardExpectation = "Mid‑table finish";
    int m_minimumLeaguePosition = 12;

    // ─── LEGEND THRESHOLD HELPERS (DYNAMIC, SCALED BY CLUB LEVEL) ───
    int getLegendAppearanceThreshold() const;
    int getLegendGoalThreshold(const std::string& position) const;
    int getLegendAssistThreshold() const;
    int getLegendCleanSheetThreshold() const;
    int getLegendTrophyThreshold() const;
    int getLegendSeasonThreshold() const;

public:
    Team(const std::string& name, const std::string& country, int level);
    Team() = default;

    // ========== BASIC GETTERS / SETTERS ==========
    const std::string& getName() const { return m_name; }
    const std::string& getCountry() const { return m_country; }
    int getClubLevel() const { return m_clubLevel; }
    int getReputation() const { return m_reputation; }
    void setReputation(int r) { m_reputation = std::max(1, std::min(100, r)); }
    void setPrimaryColour(const std::string& c)   { m_primaryColour = c; }
    void setSecondaryColour(const std::string& c) { m_secondaryColour = c; }
    void setStadiumName(const std::string& n)      { m_stadiumName = n; }
    void setFoundedYear(int y)                     { m_foundedYear = y; }
    void setBoardExpectation(const std::string& e, int minPos) {
        m_boardExpectation = e; m_minimumLeaguePosition = minPos;
    }
    const std::string& getBoardExpectation() const { return m_boardExpectation; }
    int getMinimumLeaguePosition() const { return m_minimumLeaguePosition; }
    const std::string& getPrimaryColour() const    { return m_primaryColour; }
    const std::string& getSecondaryColour() const  { return m_secondaryColour; }
    const std::string& getStadiumName() const      { return m_stadiumName; }
    int getFoundedYear() const                     { return m_foundedYear; }

    // ========== SQUAD ACCESS ==========
    const std::vector<PlayerPtr>& getSeniorSquad() const { return m_seniorSquad; }
    const std::vector<PlayerPtr>& getYouthSquad()   const { return m_youthSquad; }
    const std::vector<PlayerPtr>& getAcademy()      const { return m_academy; }

    // ========== SQUAD MANAGEMENT ==========
    void addToSenior(const PlayerPtr& p);
    void addToYouth(const PlayerPtr& p);
    void addToAcademy(const PlayerPtr& p);
    bool removePlayer(const std::string& playerId);
    PlayerPtr getPlayerById(const std::string& id) const;
    std::vector<PlayerPtr> getAvailablePlayers() const;
    bool hasEnoughPlayers() const;       // ≥ 18 seniors
    bool isOverSquadLimit() const;       // > 35 seniors

    bool promoteYouthToSenior(const std::string& playerId);
    bool demoteSeniorToYouth(const std::string& playerId);
    void processAcademyGraduation(const std::vector<std::string>& offeredContracts,
                                  std::vector<PlayerPtr>& releasedPlayers);

    // ========== DEPTH CHART ==========
    void rebuildDepthChart();
    void setDepthChartOrder(const std::string& positionGroup,
                            const std::vector<std::string>& playerIds);
    std::vector<PlayerPtr> getBestXI(const std::string& formationName = "4-3-3") const;

    // ========== SEASON RECORD ==========
    void recordResult(int gf, int ga);
    void resetSeasonRecord();
    int getGamesPlayed()  const { return m_gamesPlayed; }
    int getWins()         const { return m_wins; }
    int getDraws()        const { return m_draws; }
    int getLosses()       const { return m_losses; }
    int getGoalsFor()     const { return m_goalsFor; }
    int getGoalsAgainst() const { return m_goalsAgainst; }
    int getPoints()       const { return m_points; }
    int getGoalDifference() const { return m_goalsFor - m_goalsAgainst; }

    // ========== FACILITIES ==========
    Facilities& getFacilities() { return m_facilities; }
    const Facilities& getFacilities() const { return m_facilities; }

    // ========== CLUB RECORDS ==========
    void updateRecordsAfterMatch(int ourGoals, int opponentGoals, 
                                 const std::string& opponentName);
    void updateTransferRecord(int64_t fee, bool isIncome);
    void updatePlayerAgeRecord(int age);
    void updateMostAppearances(const std::string& playerId);

    int getBiggestWinMargin() const      { return m_biggestWinMargin; }
    int getHeaviestDefeatMargin() const  { return m_heaviestDefeatMargin; }
    int getMostPointsSeason() const      { return m_mostPointsSeason; }
    int getMostGoalsSeason() const       { return m_mostGoalsSeason; }
    int64_t getHighestTransferPaid() const    { return m_highestTransferPaid; }
    int64_t getHighestTransferReceived() const { return m_highestTransferReceived; }
    int getYoungestPlayerAge() const     { return m_youngestPlayerAge; }
    int getOldestPlayerAge() const       { return m_oldestPlayerAge; }
    const std::string& getMostAppearancesPlayerId() const {
        return m_mostAppearancesPlayerId;
    }

    // ========== TROPHY CABINET ==========
    void addTrophy(const std::string& competition, int year, int tier = 0);
    int getTrophyCount(const std::string& competition) const;
    int getTotalTrophies() const;
    const std::vector<TrophyEntry>& getTrophyCabinet() const { return m_trophyCabinet; }

    // ========== ALL‑TIME TOP LISTS ==========
    void recalculateTopLists();
    const std::vector<ClubStatLeader>& getTopScorers() const    { return m_topScorers; }
    const std::vector<ClubStatLeader>& getTopAssisters() const  { return m_topAssisters; }
    const std::vector<ClubStatLeader>& getTopAppearances() const { return m_topAppearances; }

    // ========== CLUB LEGENDS ==========
    bool isLegend(const std::string& playerId) const;
    void checkAndInductLegends(const std::map<std::string, PlayerPtr>& globalPlayerRegistry);
    void manuallyInductLegend(const std::string& playerId);
    const std::vector<std::string>& getClubLegends() const { return m_clubLegends; }

    // ========== HEAD‑TO‑HEAD ==========
    const std::map<std::string, HeadToHeadRecord>& getAllHeadToHead() const {
        return m_headToHead;
    }
    HeadToHeadRecord getHeadToHead(const std::string& opponentName) const;
    void updateHeadToHead(const std::string& opponentName, int ourGoals, int opponentGoals,
                          const std::vector<PlayerPtr>& ourPlayers,
                          const std::map<std::string, double>& playerRatings);

    // ========== RIVALS ==========
    void addRival(const std::string& clubName) { m_rivalClubNames.push_back(clubName); }
    const std::vector<std::string>& getRivals() const { return m_rivalClubNames; }
    bool isRival(const std::string& clubName) const;

    // ========== SERIALISATION ==========
    json toJson() const;
    static std::shared_ptr<Team> fromJson(const json& j);
};

using TeamPtr = std::shared_ptr<Team>;

// ========== HEAD‑TO‑HEAD QUERY IMPLEMENTATIONS ==========
inline std::vector<PlayerH2HStats> HeadToHeadRecord::getTopScorers(int limit) const {
    std::vector<PlayerH2HStats> vec;
    for (const auto& kv : playerStats) vec.push_back(kv.second);
    std::sort(vec.begin(), vec.end(),
              [](const PlayerH2HStats& a, const PlayerH2HStats& b) {
                  return a.goals > b.goals;
              });
    if ((int)vec.size() > limit) vec.resize(limit);
    return vec;
}

inline std::vector<PlayerH2HStats> HeadToHeadRecord::getTopAssisters(int limit) const {
    std::vector<PlayerH2HStats> vec;
    for (const auto& kv : playerStats) vec.push_back(kv.second);
    std::sort(vec.begin(), vec.end(),
              [](const PlayerH2HStats& a, const PlayerH2HStats& b) {
                  return a.assists > b.assists;
              });
    if ((int)vec.size() > limit) vec.resize(limit);
    return vec;
}

inline std::vector<PlayerH2HStats> HeadToHeadRecord::getMostAppearances(int limit) const {
    std::vector<PlayerH2HStats> vec;
    for (const auto& kv : playerStats) vec.push_back(kv.second);
    std::sort(vec.begin(), vec.end(),
              [](const PlayerH2HStats& a, const PlayerH2HStats& b) {
                  return a.appearances > b.appearances;
              });
    if ((int)vec.size() > limit) vec.resize(limit);
    return vec;
}

inline std::vector<PlayerH2HStats> HeadToHeadRecord::getHighestAvgRating(
    int limit, int minApps) const {
    std::vector<PlayerH2HStats> vec;
    for (const auto& kv : playerStats) {
        if (kv.second.appearances >= minApps)
            vec.push_back(kv.second);
    }
    std::sort(vec.begin(), vec.end(),
              [](const PlayerH2HStats& a, const PlayerH2HStats& b) {
                  double avgA = a.appearances > 0 ? a.totalRating / a.appearances : 0.0;
                  double avgB = b.appearances > 0 ? b.totalRating / b.appearances : 0.0;
                  return avgA > avgB;
              });
    if ((int)vec.size() > limit) vec.resize(limit);
    return vec;
}
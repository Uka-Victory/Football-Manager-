// League.hpp
#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Team.hpp"

using json = nlohmann::json;

// ========== FIXTURE ==========
struct Fixture {
    std::string date;           // "YYYY-MM-DD"
    TeamPtr homeTeam;
    TeamPtr awayTeam;
    int homeGoals = -1;         // -1 = not played
    int awayGoals = -1;
    bool played = false;
};

// ========== LEAGUE CLASS ==========
class League {
private:
    std::string m_name;
    std::string m_country;
    int m_tier = 1;                        // 1 = top division
    int m_roundsPerOpponent = 2;           // 2 = home & away
    int m_promotionSpots = 3;
    int m_relegationSpots = 3;

    std::vector<TeamPtr> m_teams;
    std::map<std::string, std::vector<Fixture>> m_fixtures;   // date -> matches

    // Season tracking
    int m_seasonYear = 2025;
    bool m_seasonComplete = false;

public:
    League() = default;
    League(const std::string& name, const std::string& country, int tier);

    // ========== BASIC GETTERS ==========
    const std::string& getName() const { return m_name; }
    const std::string& getCountry() const { return m_country; }
    int getTier() const { return m_tier; }
    int getSeasonYear() const { return m_seasonYear; }
    bool isSeasonComplete() const { return m_seasonComplete; }
    const std::vector<TeamPtr>& getTeams() const { return m_teams; }
    const std::map<std::string, std::vector<Fixture>>& getFixtures() const { return m_fixtures; }
    void setPromotionSpots(int n) { m_promotionSpots = n; }
    void setRelegationSpots(int n) { m_relegationSpots = n; }
    int getPromotionSpots() const { return m_promotionSpots; }
    int getRelegationSpots() const { return m_relegationSpots; }

    // ========== TEAM MANAGEMENT ==========
    void addTeam(const TeamPtr& team);
    void removeTeam(const TeamPtr& team);

    // ========== FIXTURE GENERATION ==========
    void generateSchedule(const std::string& startDate);
    void generateSchedule(int year, int month, int day);

    // ========== MATCH DAY PROCESSING ==========
    std::vector<Fixture> getFixturesForDate(const std::string& date) const;
    void recordMatchResult(const std::string& date, const Fixture& result);
    std::vector<Fixture> getAllUnplayedFixtures() const;
    void markSeasonComplete();

    // ========== LEAGUE TABLE ==========
    std::vector<TeamPtr> getSortedTable() const;
    void printTable() const;

    // ========== END‑OF‑SEASON ==========
    void endSeason(League* lowerLeague = nullptr);
    void resetAllTeamRecords();

    // ========== DATE HELPERS ==========
    static std::string addDays(const std::string& date, int days);
    static int daysBetween(const std::string& d1, const std::string& d2);

    // ========== SERIALISATION ==========
    json toJson() const;
    static std::shared_ptr<League> fromJson(const json& j,
        const std::map<std::string, TeamPtr>& teamRegistry);
};

using LeaguePtr = std::shared_ptr<League>;
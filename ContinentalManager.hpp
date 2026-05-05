// ContinentalManager.hpp
#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "League.hpp"
#include "Team.hpp"
#include "Player.hpp"
#include "GameCalendar.hpp"

// ========== COEFFICIENT DATA ==========
struct CountryCoefficient {
    std::string countryName;
    double currentSeasonPoints = 0.0;
    std::vector<double> last5SeasonsPoints;  // most recent first
    double total5Year() const;
};

struct ClubCoefficient {
    std::string clubName;
    double currentSeasonPoints = 0.0;
    std::vector<double> last5SeasonsPoints;
    double total5Year() const;
};

// ========== CONTINENTAL COMPETITION ==========
enum class Continent { Europe, SouthAmerica, NorthAmerica, Asia, Africa, Oceania };

class ContinentalManager {
public:
    ContinentalManager(Continent continent);

    // ========== COEFFICIENTS ==========
    void updateCountryCoefficients(const std::vector<LeaguePtr>& leagues,
                                   const std::map<std::string, TeamPtr>& teams);
    void updateClubCoefficients(const std::map<std::string, TeamPtr>& teams);
    std::vector<std::string> getQualifiedClubs(const std::string& competition,
                                                const std::vector<LeaguePtr>& leagues,
                                                int seasonYear);
    int getQualificationSpots(const std::string& country, const std::string& competition) const;

    // ========== COMPETITION SETUP ==========
    void setupChampionsLeague(const std::vector<std::string>& qualifiedClubs,
                          const std::map<std::string, std::shared_ptr<Team>>& teamMap,
                          GameCalendar& calendar);

void setupEuropaLeague(const std::vector<std::string>& qualifiedClubs,
                       const std::map<std::string, std::shared_ptr<Team>>& teamMap,
                       GameCalendar& calendar);

void setupConferenceLeague(const std::vector<std::string>& qualifiedClubs,
                           const std::map<std::string, std::shared_ptr<Team>>& teamMap,
                           GameCalendar& calendar);

    // ========== ACCESSORS ==========
    const std::map<std::string, CountryCoefficient>& getCountryCoefficients() const;
    const std::map<std::string, ClubCoefficient>& getClubCoefficients() const;
    const std::vector<Fixture>& getLeaguePhaseFixtures(const std::string& competition) const;
    std::vector<std::string> getLeaguePhaseTable(const std::string& competition) const;

    // ========== PERSISTENCE ==========
    json toJson() const;
    void fromJson(const json& j);

private:
    Continent m_continent;
    std::map<std::string, CountryCoefficient> m_countryCoefficients;
    std::map<std::string, ClubCoefficient> m_clubCoefficients;

    // For each competition, we store the league‑phase fixtures and results
    std::map<std::string, std::vector<Fixture>> m_fixtures;   // "UCL", "UEL", "UECL"
    std::map<std::string, std::map<std::string, int>> m_points;   // competition -> clubName -> points
    std::map<std::string, std::map<std::string, int>> m_goalDiff; // competition -> clubName -> GD

    // Helper to create a Swiss‑style schedule for 36 teams
    void generateSwissSchedule(const std::string& competition,
                               const std::vector<TeamPtr>& clubs,
                               const std::string& startDate,
                               GameCalendar& calendar);
};
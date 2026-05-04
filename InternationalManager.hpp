// InternationalManager.hpp
#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Player.hpp"
#include "Team.hpp"
#include "GameCalendar.hpp"
#include "MatchEngine.hpp"

// ========== TOURNAMENTS ==========
enum class InternationalTournament {
    WorldCup,
    UEFAEuros,
    CopaAmerica,
    AFCON,
    AsianCup,
    GoldCup,
    OFCNationsCup
};

// ========== CONFEDERATIONS ==========
enum class Confederation {
    UEFA,
    CONMEBOL,
    CONCACAF,
    CAF,
    AFC,
    OFC
};

// ========== NATIONAL TEAM ==========
struct NationalTeam {
    std::string country;
    std::vector<std::string> playerIds;   // 23 players
    std::string managerId;
    int worldRanking = 50;
    Confederation confederation = Confederation::UEFA;
};

// ========== QUALIFYING FIXTURE ==========
struct QualifyingFixture {
    std::string date;
    std::string homeCountry;
    std::string awayCountry;
    int homeGoals = -1;
    int awayGoals = -1;
    bool played = false;
    std::string group;       // "Group A", etc.
    int matchday = 0;        // 1-10 typically
};

// ========== TOURNAMENT FIXTURE ==========
struct TournamentFixture {
    std::string date;
    std::string homeCountry;
    std::string awayCountry;
    int homeGoals = -1;
    int awayGoals = -1;
    bool played = false;
    std::string stage;       // "Group", "Round of 16", "QF", "SF", "Final"
    std::string group;       // "A"-"L"
};

class InternationalManager {
private:
    std::map<std::string, NationalTeam> m_nationalTeams;
    std::map<InternationalTournament, std::vector<QualifyingFixture>> m_qualifyingFixtures;
    std::map<InternationalTournament, std::vector<TournamentFixture>> m_tournamentFixtures;
    std::map<InternationalTournament, std::map<std::string, int>> m_groupPoints;       // tournament -> country -> points
    std::map<InternationalTournament, std::map<std::string, int>> m_groupGoalDiff;
    std::map<InternationalTournament, std::map<std::string, int>> m_qualifyingPoints;   // tournament -> country -> points
    std::map<InternationalTournament, std::map<std::string, int>> m_qualifyingGoalDiff;

    int m_currentYear = 2025;
    bool m_isWorldCupYear = false;
    bool m_isEurosYear = false;
    bool m_isCopaAmericaYear = false;
    bool m_isAFCONYear = false;

    // Pre‑computed qualified lists for the current cycle
    std::vector<std::string> m_worldCupQualified;
    std::vector<std::string> m_eurosQualified;
    std::vector<std::string> m_copaAmericaQualified;
    std::vector<std::string> m_afconQualified;

    // Host countries (set once per cycle)
    std::string m_worldCupHost;
    std::string m_eurosHost;

    // ========== PRIVATE HELPERS ==========
    Confederation getConfederation(const std::string& country) const;
    std::vector<std::string> getCountriesInConfederation(Confederation conf) const;
    void generateQualifyingGroups(InternationalTournament tournament,
                                  const std::vector<std::string>& countries,
                                  int groups, int teamsPerGroup,
                                  GameCalendar& calendar);
    std::vector<std::string> getGroupWinners(InternationalTournament tournament,
                                              const std::string& groupPrefix,
                                              int numGroups) const;
    std::vector<std::string> getGroupRunnersUp(InternationalTournament tournament,
                                                const std::string& groupPrefix,
                                                int numGroups) const;

public:
    InternationalManager();

    // ========== NATIONAL TEAM MANAGEMENT ==========
    void generateNationalTeams(const std::map<std::string, PlayerPtr>& allPlayers,
                               const std::vector<std::string>& countries);
    void selectNationalSquad(const std::string& country,
                             const std::map<std::string, PlayerPtr>& allPlayers);
    NationalTeam* getNationalTeam(const std::string& country);
    void updateWorldRankings();

    // ========== QUALIFICATION ==========
    void setupWorldCupQualifiers(GameCalendar& calendar);
    void setupEurosQualifiers(GameCalendar& calendar);
    void processQualifyingMatchday(InternationalTournament tournament,
                                   const std::string& date);
    std::vector<std::string> getQualifiedCountries(InternationalTournament tournament) const;
    const std::vector<QualifyingFixture>& getQualifyingFixtures(
        InternationalTournament tournament) const;

    // ========== FINALS TOURNAMENT ==========
    void setupTournament(InternationalTournament tournament,
                         const std::vector<std::string>& participatingCountries,
                         GameCalendar& calendar);
    void setupWorldCup(GameCalendar& calendar);
    void setupEuros(GameCalendar& calendar);
    void setupCopaAmerica(GameCalendar& calendar);
    void setupAFCON(GameCalendar& calendar);
    const std::vector<TournamentFixture>& getTournamentFixtures(
        InternationalTournament tournament) const;
    void recordTournamentResult(InternationalTournament tournament,
                                const std::string& homeCountry,
                                const std::string& awayCountry,
                                int homeGoals, int awayGoals,
                                const std::string& date);
    std::map<std::string, int> getGroupStandings(InternationalTournament tournament,
                                                  const std::string& group) const;

    // ========== INTERNATIONAL BREAK ==========
    bool isInternationalBreak(const std::string& date) const;
    std::vector<std::string> getCalledUpPlayers(const std::string& country) const;

    // ========== YEAR CYCLE ==========
    void setYear(int year);
    bool isTournamentYear(InternationalTournament tournament) const;
    int getCurrentYear() const { return m_currentYear; }

    // ========== PERSISTENCE ==========
    json toJson() const;
    void fromJson(const json& j);
};
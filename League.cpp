// League.cpp
#include "League.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <cmath>
#include <set>

// ========== CONSTRUCTOR ==========
League::League(const std::string& name, const std::string& country, int tier)
    : m_name(name), m_country(country), m_tier(tier) {
}

// ========== TEAM MANAGEMENT ==========
void League::addTeam(const TeamPtr& team) {
    if (!team) return;
    // Avoid duplicates
    for (auto& t : m_teams)
        if (t->getName() == team->getName()) return;
    m_teams.push_back(team);
}

void League::removeTeam(const TeamPtr& team) {
    auto it = std::find(m_teams.begin(), m_teams.end(), team);
    if (it != m_teams.end()) m_teams.erase(it);
}

// ========== FIXTURE GENERATION ==========
void League::generateSchedule(const std::string& startDate) {
    m_fixtures.clear();
    m_seasonComplete = false;

    if (m_teams.size() < 2) return;

    // Circle method for round‑robin
    std::vector<TeamPtr> teams = m_teams;
    // Shuffle for variety each season
    std::shuffle(teams.begin(), teams.end(), Utils::getRng());

    // If odd number of teams, add a bye (nullptr)
    bool hasBye = false;
    if (teams.size() % 2 != 0) {
        teams.push_back(nullptr);
        hasBye = true;
    }

    int n = teams.size();          // even, possibly with bye
    int half = n / 2;
    int totalRounds = (n - 1) * m_roundsPerOpponent;

    std::string currentDate = startDate;
    const int daysBetweenMatchdays = 7;   // weekly matches

    for (int round = 0; round < totalRounds; ++round) {
        int roundIndex = round % (n - 1);
        bool secondHalf = (round >= (n - 1));   // return leg for second half of season

        // Rotate: keep team[0] fixed, rotate the rest
        std::vector<TeamPtr> rotated = teams;
        for (int r = 0; r < roundIndex; ++r) {
            // Move last element to index 1, shift the rest right
            TeamPtr last = rotated.back();
            for (int i = n - 1; i > 1; --i)
                rotated[i] = rotated[i - 1];
            rotated[1] = last;
        }

        std::vector<Fixture> roundFixtures;
        for (int i = 0; i < half; ++i) {
            TeamPtr home = rotated[i];
            TeamPtr away = rotated[n - 1 - i];

            if (!home || !away) continue;   // bye, skip

            // For the second half of the season, swap home/away
            if (secondHalf) std::swap(home, away);

            Fixture fix;
            fix.date = currentDate;
            fix.homeTeam = home;
            fix.awayTeam = away;
            fix.played = false;
            fix.homeGoals = -1;
            fix.awayGoals = -1;
            roundFixtures.push_back(fix);
        }

        m_fixtures[currentDate] = roundFixtures;
        currentDate = addDays(currentDate, daysBetweenMatchdays);
    }
}

void League::generateSchedule(int year, int month, int day) {
    std::ostringstream oss;
    oss << year << "-" << std::setfill('0') << std::setw(2) << month
        << "-" << std::setw(2) << day;
    generateSchedule(oss.str());
}

// ========== MATCH DAY PROCESSING ==========
std::vector<Fixture> League::getFixturesForDate(const std::string& date) const {
    auto it = m_fixtures.find(date);
    if (it != m_fixtures.end()) return it->second;
    return {};
}

void League::recordMatchResult(const std::string& date, const Fixture& result) {
    auto it = m_fixtures.find(date);
    if (it != m_fixtures.end()) {
        for (auto& fix : it->second) {
            if (fix.homeTeam == result.homeTeam && fix.awayTeam == result.awayTeam) {
                fix.homeGoals = result.homeGoals;
                fix.awayGoals = result.awayGoals;
                fix.played = true;
                return;
            }
        }
    }
    // If fixture not found (e.g., cup match), do nothing
}

std::vector<Fixture> League::getAllUnplayedFixtures() const {
    std::vector<Fixture> unplayed;
    for (const auto& kv : m_fixtures) {
        for (const auto& fix : kv.second) {
            if (!fix.played) unplayed.push_back(fix);
        }
    }
    return unplayed;
}

void League::markSeasonComplete() {
    m_seasonComplete = true;
}

// ========== LEAGUE TABLE ==========
std::vector<TeamPtr> League::getSortedTable() const {
    std::vector<TeamPtr> sorted = m_teams;
    std::sort(sorted.begin(), sorted.end(),
        [](const TeamPtr& a, const TeamPtr& b) {
            int pa = a->getPoints();
            int pb = b->getPoints();
            if (pa != pb) return pa > pb;

            int gda = a->getGoalDifference();
            int gdb = b->getGoalDifference();
            if (gda != gdb) return gda > gdb;

            if (a->getGoalsFor() != b->getGoalsFor())
                return a->getGoalsFor() > b->getGoalsFor();

            return a->getName() < b->getName();
        });
    return sorted;
}

void League::printTable() const {
    auto table = getSortedTable();
    std::cout << "\n=== " << m_name << " (Tier " << m_tier << ") ===\n";
    printf("%3s %-22s %2s %2s %2s %2s %3s %3s %3s %3s\n",
           "Pos", "Team", "P", "W", "D", "L", "GF", "GA", "GD", "Pts");
    for (size_t i = 0; i < table.size(); ++i) {
        auto& t = table[i];
        int gd = t->getGoalDifference();
        printf("%3zu %-22s %2d %2d %2d %2d %3d %3d %3d %3d\n",
               i + 1, t->getName().c_str(),
               t->getGamesPlayed(), t->getWins(), t->getDraws(), t->getLosses(),
               t->getGoalsFor(), t->getGoalsAgainst(), gd, t->getPoints());
    }
}

// ========== END‑OF‑SEASON ==========
void League::endSeason(League* lowerLeague) {
    m_seasonComplete = true;

    auto table = getSortedTable();
    std::cout << "\n===== " << m_name << " SEASON ENDED =====\n";
    if (!table.empty()) {
        std::cout << "Champion: " << table[0]->getName() << "\n";
        // Record in history (will be done by WorldHistory)
    }

    // Promotion / Relegation
    if (lowerLeague && !table.empty() && !lowerLeague->m_teams.empty()) {
        auto lowerTable = lowerLeague->getSortedTable();
        int promo = std::min(m_relegationSpots, (int)lowerTable.size());
        int releg = std::min(m_relegationSpots, (int)table.size());

        // For simplicity, don't promote fewer than relegation spots
        promo = std::min(promo, releg);
        releg = promo;

        for (int i = 0; i < promo; ++i) {
            TeamPtr promotedTeam = lowerTable[i];
            TeamPtr relegatedTeam = table[table.size() - 1 - i];

            // Remove from old leagues
            auto& upper = m_teams;
            auto& lower = lowerLeague->m_teams;
            upper.erase(std::remove(upper.begin(), upper.end(), relegatedTeam), upper.end());
            lower.erase(std::remove(lower.begin(), lower.end(), promotedTeam), lower.end());

            // Add to new leagues
            upper.push_back(promotedTeam);
            lower.push_back(relegatedTeam);

            std::cout << promotedTeam->getName() << " promoted to " << m_name << "\n";
            std::cout << relegatedTeam->getName() << " relegated to " << lowerLeague->getName() << "\n";
        }
    }

    // Reset records
    resetAllTeamRecords();
}

void League::resetAllTeamRecords() {
    for (auto& team : m_teams) {
        team->resetSeasonRecord();
    }
}

// ========== DATE HELPERS (using humble arithmetic) ==========
std::string League::addDays(const std::string& date, int days) {
    int year, month, day;
    char dummy;
    std::istringstream ss(date);
    ss >> year >> dummy >> month >> dummy >> day;

    // Days in each month (leap year handled)
    static const int monthDays[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    day += days;
    while (day > monthDays[month - 1] + ((month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) ? 1 : 0)) {
        day -= monthDays[month - 1] + ((month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) ? 1 : 0);
        month++;
        if (month > 12) {
            month = 1;
            year++;
        }
    }

    std::ostringstream oss;
    oss << year << "-" << std::setfill('0') << std::setw(2) << month
        << "-" << std::setw(2) << day;
    return oss.str();
}

int League::daysBetween(const std::string& d1, const std::string& d2) {
    // Placeholder: compute difference in days (not urgent for fixture logic)
    return 0;
}

// ========== SERIALISATION ==========
json League::toJson() const {
    json j;
    j["name"] = m_name;
    j["country"] = m_country;
    j["tier"] = m_tier;
    j["roundsPerOpponent"] = m_roundsPerOpponent;
    j["promotionSpots"] = m_promotionSpots;
    j["relegationSpots"] = m_relegationSpots;
    j["seasonYear"] = m_seasonYear;
    j["seasonComplete"] = m_seasonComplete;

    // Teams — store names only (full objects saved elsewhere)
    j["teams"] = json::array();
    for (auto& t : m_teams) j["teams"].push_back(t->getName());

    // Fixtures
    j["fixtures"] = json::object();
    for (const auto& kv : m_fixtures) {
        json arr = json::array();
        for (const auto& fix : kv.second) {
            arr.push_back({
                {"homeTeam", fix.homeTeam ? fix.homeTeam->getName() : ""},
                {"awayTeam", fix.awayTeam ? fix.awayTeam->getName() : ""},
                {"homeGoals", fix.homeGoals},
                {"awayGoals", fix.awayGoals},
                {"played", fix.played}
            });
        }
        j["fixtures"][kv.first] = arr;
    }

    return j;
}

std::shared_ptr<League> League::fromJson(const json& j,
    const std::map<std::string, TeamPtr>& teamRegistry) {
    auto league = std::make_shared<League>();
    league->m_name = j.at("name").get<std::string>();
    league->m_country = j.at("country").get<std::string>();
    league->m_tier = j.value("tier", 1);
    league->m_roundsPerOpponent = j.value("roundsPerOpponent", 2);
    league->m_promotionSpots = j.value("promotionSpots", 3);
    league->m_relegationSpots = j.value("relegationSpots", 3);
    league->m_seasonYear = j.value("seasonYear", 2025);
    league->m_seasonComplete = j.value("seasonComplete", false);

    // Restore teams from registry
    league->m_teams.clear();
    if (j.contains("teams") && j["teams"].is_array()) {
        for (const auto& name : j["teams"]) {
            auto it = teamRegistry.find(name.get<std::string>());
            if (it != teamRegistry.end())
                league->m_teams.push_back(it->second);
        }
    }

    // Restore fixtures
    league->m_fixtures.clear();
    if (j.contains("fixtures") && j["fixtures"].is_object()) {
        for (auto it = j["fixtures"].begin(); it != j["fixtures"].end(); ++it) {
            std::vector<Fixture> fixList;
            for (const auto& fj : it.value()) {
                Fixture fix;
                fix.date = it.key();
                fix.homeGoals = fj.value("homeGoals", -1);
                fix.awayGoals = fj.value("awayGoals", -1);
                fix.played = fj.value("played", false);

                // Resolve team pointers from registry
                std::string homeName = fj.value("homeTeam", "");
                std::string awayName = fj.value("awayTeam", "");
                auto hit = teamRegistry.find(homeName);
                auto ait = teamRegistry.find(awayName);
                if (hit != teamRegistry.end()) fix.homeTeam = hit->second;
                if (ait != teamRegistry.end()) fix.awayTeam = ait->second;
                fixList.push_back(fix);
            }
            league->m_fixtures[it.key()] = fixList;
        }
    }

    return league;
}
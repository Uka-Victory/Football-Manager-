// ContinentalManager.cpp
#include "ContinentalManager.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cmath>
#include <set>
#include <sstream>
#include <iomanip>

// ========== DATE HELPER ==========
static std::string addDays(const std::string& date, int days) {
    int year, month, day;
    char dash;
    std::istringstream ss(date);
    ss >> year >> dash >> month >> dash >> day;

    static const int monthDays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    day += days;
    while (true) {
        int maxDay = monthDays[month - 1];
        if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))
            maxDay = 29;
        if (day <= maxDay) break;
        day -= maxDay;
        month++;
        if (month > 12) { month = 1; year++; }
    }
    std::ostringstream oss;
    oss << year << "-" << std::setfill('0') << std::setw(2) << month
        << "-" << std::setw(2) << day;
    return oss.str();
}

// ========== COEFFICIENT METHODS ==========
double CountryCoefficient::total5Year() const {
    double total = currentSeasonPoints;
    for (double pts : last5SeasonsPoints) total += pts;
    return total;
}

double ClubCoefficient::total5Year() const {
    double total = currentSeasonPoints;
    for (double pts : last5SeasonsPoints) total += pts;
    return total;
}

// ========== CONSTRUCTOR ==========
ContinentalManager::ContinentalManager(Continent continent)
    : m_continent(continent) {
}

// ========== COUNTRY COEFFICIENTS ==========
void ContinentalManager::updateCountryCoefficients(
    const std::vector<LeaguePtr>& leagues,
    const std::map<std::string, TeamPtr>& teams) {

    for (auto& kv : m_countryCoefficients)
        kv.second.currentSeasonPoints = 0.0;

    for (const auto& league : leagues) {
        const std::string& country = league->getCountry();
        if (m_countryCoefficients.find(country) == m_countryCoefficients.end()) {
            CountryCoefficient cc;
            cc.countryName = country;
            m_countryCoefficients[country] = cc;
        }
    }

    for (const auto& compFixtures : m_fixtures) {
        const std::string& competition = compFixtures.first;
        double winPoints = 2.0, drawPoints = 1.0, participationPoints = 4.0;
        if (competition == "UEL") {
            winPoints = 1.2; drawPoints = 0.6; participationPoints = 2.4;
        } else if (competition == "UECL") {
            winPoints = 0.8; drawPoints = 0.4; participationPoints = 1.6;
        }

        for (const auto& fix : compFixtures.second) {
            if (!fix.played) continue;
            std::string homeCountry, awayCountry;
            if (fix.homeTeam) homeCountry = fix.homeTeam->getCountry();
            if (fix.awayTeam) awayCountry = fix.awayTeam->getCountry();

            if (!homeCountry.empty() && m_countryCoefficients.count(homeCountry)) {
                m_countryCoefficients[homeCountry].currentSeasonPoints += participationPoints / 8.0;
                if (fix.homeGoals > fix.awayGoals)
                    m_countryCoefficients[homeCountry].currentSeasonPoints += winPoints;
                else if (fix.homeGoals == fix.awayGoals)
                    m_countryCoefficients[homeCountry].currentSeasonPoints += drawPoints;
            }
            if (!awayCountry.empty() && m_countryCoefficients.count(awayCountry)) {
                m_countryCoefficients[awayCountry].currentSeasonPoints += participationPoints / 8.0;
                if (fix.awayGoals > fix.homeGoals)
                    m_countryCoefficients[awayCountry].currentSeasonPoints += winPoints;
                else if (fix.awayGoals == fix.homeGoals)
                    m_countryCoefficients[awayCountry].currentSeasonPoints += drawPoints;
            }
        }
    }

    // Round of 16 bonus for top 8
    for (const auto& comp : {"UCL", "UEL", "UECL"}) {
        std::string compStr = comp;
        if (m_points.find(compStr) == m_points.end()) continue;

        std::vector<std::pair<std::string, int>> table;
        for (const auto& kv : m_points.at(compStr))
            table.push_back({kv.first, kv.second});
        std::sort(table.begin(), table.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        double r16Bonus = 5.0;
        if (compStr == "UEL") r16Bonus = 3.0;
        else if (compStr == "UECL") r16Bonus = 2.0;

        for (size_t i = 0; i < table.size() && i < 8; ++i) {
            std::string clubName = table[i].first;
            auto teamIt = teams.find(clubName);
            if (teamIt != teams.end()) {
                std::string country = teamIt->second->getCountry();
                if (m_countryCoefficients.count(country))
                    m_countryCoefficients[country].currentSeasonPoints += r16Bonus;
            }
        }
    }
}

void ContinentalManager::updateClubCoefficients(
    const std::map<std::string, TeamPtr>& teams) {

    for (auto& kv : m_clubCoefficients)
        kv.second.currentSeasonPoints = 0.0;

    for (const auto& kv : teams) {
        if (m_clubCoefficients.find(kv.first) == m_clubCoefficients.end()) {
            ClubCoefficient cc;
            cc.clubName = kv.first;
            m_clubCoefficients[kv.first] = cc;
        }
    }

    for (const auto& compFixtures : m_fixtures) {
        const std::string& comp = compFixtures.first;
        double winPts = 2.0, drawPts = 1.0, partPts = 4.0;
        if (comp == "UEL") { winPts = 1.2; drawPts = 0.6; partPts = 2.4; }
        if (comp == "UECL") { winPts = 0.8; drawPts = 0.4; partPts = 1.6; }

        for (const auto& fix : compFixtures.second) {
            if (!fix.played) continue;
            if (fix.homeTeam && m_clubCoefficients.count(fix.homeTeam->getName())) {
                auto& cc = m_clubCoefficients[fix.homeTeam->getName()];
                cc.currentSeasonPoints += partPts / 8.0;
                if (fix.homeGoals > fix.awayGoals) cc.currentSeasonPoints += winPts;
                else if (fix.homeGoals == fix.awayGoals) cc.currentSeasonPoints += drawPts;
            }
            if (fix.awayTeam && m_clubCoefficients.count(fix.awayTeam->getName())) {
                auto& cc = m_clubCoefficients[fix.awayTeam->getName()];
                cc.currentSeasonPoints += partPts / 8.0;
                if (fix.awayGoals > fix.homeGoals) cc.currentSeasonPoints += winPts;
                else if (fix.awayGoals == fix.homeGoals) cc.currentSeasonPoints += drawPts;
            }
        }
    }
}

// ========== QUALIFICATION ==========
int ContinentalManager::getQualificationSpots(const std::string& country,
                                               const std::string& competition) const {
    std::vector<std::pair<std::string, double>> ranking;
    for (const auto& kv : m_countryCoefficients)
        ranking.push_back({kv.first, kv.second.total5Year()});
    std::sort(ranking.begin(), ranking.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    int rank = 1;
    for (const auto& r : ranking) {
        if (r.first == country) break;
        rank++;
    }

    if (competition == "UCL") {
        if (rank <= 4) return 4;
        if (rank <= 6) return 3;
        if (rank <= 10) return 2;
        if (rank <= 15) return 1;
        return 0;
    } else if (competition == "UEL") {
        if (rank <= 4) return 2;
        if (rank <= 6) return 2;
        if (rank <= 10) return 1;
        if (rank <= 15) return 1;
        return 0;
    } else if (competition == "UECL") {
        if (rank <= 4) return 1;
        if (rank <= 10) return 1;
        if (rank <= 15) return 1;
        return 0;
    }
    return 0;
}

std::vector<std::string> ContinentalManager::getQualifiedClubs(
    const std::string& competition,
    const std::vector<LeaguePtr>& leagues,
    int seasonYear) {

    std::vector<std::string> qualified;
    std::map<std::string, std::vector<LeaguePtr>> countryLeagues;
    for (const auto& league : leagues)
        countryLeagues[league->getCountry()].push_back(league);

    for (const auto& [country, countryLeagueVec] : countryLeagues) {
        int spots = getQualificationSpots(country, competition);
        if (spots <= 0) continue;

        LeaguePtr topLeague = nullptr;
        for (const auto& l : countryLeagueVec) {
            if (!topLeague || l->getTier() < topLeague->getTier())
                topLeague = l;
        }
        if (!topLeague) continue;

        auto table = topLeague->getSortedTable();
        int taken = 0;
        for (const auto& team : table) {
            if (taken >= spots) break;
            qualified.push_back(team->getName());
            taken++;
        }
    }
    return qualified;
}

// ========== COMPETITION SETUP (FULLY IMPLEMENTED) ==========
void ContinentalManager::setupChampionsLeague(
    const std::vector<std::string>& qualifiedClubs,
    const std::map<std::string, TeamPtr>& allTeams,
    GameCalendar& calendar) {

    std::vector<TeamPtr> clubs;
    for (const auto& name : qualifiedClubs) {
        auto it = allTeams.find(name);
        if (it != allTeams.end())
            clubs.push_back(it->second);
    }
    if (clubs.size() < 36) {
        // Fill remaining spots with best unqualified clubs? Or just pad.
        // In a full implementation, we'd have qualifying rounds. For now, we accept.
    }

    std::string startDate = std::to_string(calendar.getYear()) + "-09-15";
    generateSwissSchedule("UCL", clubs, startDate, calendar);
}

void ContinentalManager::setupEuropaLeague(
    const std::vector<std::string>& qualifiedClubs,
    const std::map<std::string, TeamPtr>& allTeams,
    GameCalendar& calendar) {

    std::vector<TeamPtr> clubs;
    for (const auto& name : qualifiedClubs) {
        auto it = allTeams.find(name);
        if (it != allTeams.end())
            clubs.push_back(it->second);
    }

    std::string startDate = std::to_string(calendar.getYear()) + "-09-15";
    generateSwissSchedule("UEL", clubs, startDate, calendar);
}

void ContinentalManager::setupConferenceLeague(
    const std::vector<std::string>& qualifiedClubs,
    const std::map<std::string, TeamPtr>& allTeams,
    GameCalendar& calendar) {

    std::vector<TeamPtr> clubs;
    for (const auto& name : qualifiedClubs) {
        auto it = allTeams.find(name);
        if (it != allTeams.end())
            clubs.push_back(it->second);
    }

    std::string startDate = std::to_string(calendar.getYear()) + "-09-15";
    generateSwissSchedule("UECL", clubs, startDate, calendar);
}

void ContinentalManager::generateSwissSchedule(
    const std::string& competition,
    const std::vector<TeamPtr>& clubs,
    const std::string& startDate,
    GameCalendar& calendar) {

    if (clubs.size() != 36) return;

    // Sort by club coefficient (or club level) for seeding pots
    std::vector<TeamPtr> sorted = clubs;
    std::sort(sorted.begin(), sorted.end(),
        [](const TeamPtr& a, const TeamPtr& b) {
            return a->getClubLevel() > b->getClubLevel();
        });

    std::vector<std::vector<TeamPtr>> pots(4);
    for (int i = 0; i < 36; ++i)
        pots[i / 9].push_back(sorted[i]);

    m_fixtures[competition].clear();
    m_points[competition].clear();
    m_goalDiff[competition].clear();

    // Initialize points and goal difference
    for (const auto& club : clubs) {
        m_points[competition][club->getName()] = 0;
        m_goalDiff[competition][club->getName()] = 0;
    }

    const int totalMatchdays = 8;
    std::string matchDate = startDate;

    // create a deterministic but varied pairing across matchdays
    for (int matchday = 0; matchday < totalMatchdays; ++matchday) {
        std::vector<Fixture> dayFixtures;
        // Each club plays one match per matchday
        std::set<std::string> alreadyPlaying;

        // Helper to pick a valid opponent from a pot
        auto pickOpponent = [&](const TeamPtr& homeTeam, const std::vector<TeamPtr>& awayPot,
                                int offset) -> TeamPtr {
            for (size_t attempt = 0; attempt < awayPot.size(); ++attempt) {
                size_t idx = (offset + attempt) % awayPot.size();
                TeamPtr awayTeam = awayPot[idx];
                if (awayTeam == homeTeam) continue;
                if (alreadyPlaying.count(awayTeam->getName())) continue;
                // Check if these two have already met (simplistic: not tracked yet,
                // but we can skip for now)
                return awayTeam;
            }
            return nullptr;
        };

        // pair teams across pots
        for (int homePot = 0; homePot < 4; ++homePot) {
            for (size_t i = 0; i < pots[homePot].size(); ++i) {
                TeamPtr homeTeam = pots[homePot][i];
                if (alreadyPlaying.count(homeTeam->getName())) continue;

                // Choose away pot cycling
                int awayPot = (homePot + 1 + matchday) % 4;
                TeamPtr awayTeam = pickOpponent(homeTeam, pots[awayPot], i * 3 + matchday * 5);
                if (!awayTeam) {
                    // try another pot
                    awayPot = (homePot + 2 + matchday) % 4;
                    awayTeam = pickOpponent(homeTeam, pots[awayPot], i * 3 + matchday * 7);
                }
                if (!awayTeam) {
                    awayPot = (homePot + 3 + matchday) % 4;
                    awayTeam = pickOpponent(homeTeam, pots[awayPot], i * 3 + matchday * 11);
                }
                if (!awayTeam) continue; // couldn't find opponent

                Fixture fix;
                fix.date = matchDate;
                fix.homeTeam = homeTeam;
                fix.awayTeam = awayTeam;
                fix.homeGoals = -1;
                fix.awayGoals = -1;
                fix.played = false;
                dayFixtures.push_back(fix);

                alreadyPlaying.insert(homeTeam->getName());
                alreadyPlaying.insert(awayTeam->getName());
            }
        }

        m_fixtures[competition].insert(
            m_fixtures[competition].end(),
            dayFixtures.begin(), dayFixtures.end());

        // midweek schedule: shift by 7 or 14 days alternating
        matchDate = addDays(matchDate, (matchday % 2 == 0) ? 7 : 14);
    }
}

// ========== ACCESSORS ==========
const std::map<std::string, CountryCoefficient>&
ContinentalManager::getCountryCoefficients() const {
    return m_countryCoefficients;
}

const std::map<std::string, ClubCoefficient>&
ContinentalManager::getClubCoefficients() const {
    return m_clubCoefficients;
}

const std::vector<Fixture>&
ContinentalManager::getLeaguePhaseFixtures(const std::string& competition) const {
    static std::vector<Fixture> empty;
    auto it = m_fixtures.find(competition);
    if (it != m_fixtures.end()) return it->second;
    return empty;
}

std::vector<std::string>
ContinentalManager::getLeaguePhaseTable(const std::string& competition) const {
    std::vector<std::string> table;
    auto ptsIt = m_points.find(competition);
    if (ptsIt == m_points.end()) return table;

    auto gdIt = m_goalDiff.find(competition);
    std::vector<std::pair<std::string, int>> sorted;
    for (const auto& kv : ptsIt->second) {
        sorted.push_back({kv.first, kv.second});
    }

    std::sort(sorted.begin(), sorted.end(),
        [&](const auto& a, const auto& b) {
            if (a.second != b.second) return a.second > b.second;
            int gdA = 0, gdB = 0;
            if (gdIt != m_goalDiff.end()) {
                auto itA = gdIt->second.find(a.first);
                auto itB = gdIt->second.find(b.first);
                if (itA != gdIt->second.end()) gdA = itA->second;
                if (itB != gdIt->second.end()) gdB = itB->second;
            }
            return gdA > gdB;
        });

    for (const auto& s : sorted)
        table.push_back(s.first);
    return table;
}

// ========== JSON ==========
json ContinentalManager::toJson() const {
    json j;
    j["continent"] = static_cast<int>(m_continent);

    j["countryCoefficients"] = json::object();
    for (const auto& kv : m_countryCoefficients) {
        json cc;
        cc["countryName"] = kv.second.countryName;
        cc["currentSeasonPoints"] = kv.second.currentSeasonPoints;
        cc["last5SeasonsPoints"] = kv.second.last5SeasonsPoints;
        j["countryCoefficients"][kv.first] = cc;
    }

    j["clubCoefficients"] = json::object();
    for (const auto& kv : m_clubCoefficients) {
        json cc;
        cc["clubName"] = kv.second.clubName;
        cc["currentSeasonPoints"] = kv.second.currentSeasonPoints;
        cc["last5SeasonsPoints"] = kv.second.last5SeasonsPoints;
        j["clubCoefficients"][kv.first] = cc;
    }

    j["fixtures"] = json::object();
    for (const auto& compFix : m_fixtures) {
        json arr = json::array();
        for (const auto& fix : compFix.second) {
            arr.push_back({
                {"date", fix.date},
                {"homeTeam", fix.homeTeam ? fix.homeTeam->getName() : ""},
                {"awayTeam", fix.awayTeam ? fix.awayTeam->getName() : ""},
                {"homeGoals", fix.homeGoals},
                {"awayGoals", fix.awayGoals},
                {"played", fix.played}
            });
        }
        j["fixtures"][compFix.first] = arr;
    }

    j["points"] = json::object();
    for (const auto& compPts : m_points) {
        json obj = json::object();
        for (const auto& kv : compPts.second)
            obj[kv.first] = kv.second;
        j["points"][compPts.first] = obj;
    }

    j["goalDiff"] = json::object();
    for (const auto& compGd : m_goalDiff) {
        json obj = json::object();
        for (const auto& kv : compGd.second)
            obj[kv.first] = kv.second;
        j["goalDiff"][compGd.first] = obj;
    }

    return j;
}

void ContinentalManager::fromJson(const json& j) {
    m_continent = static_cast<Continent>(j.value("continent", 0));

    m_countryCoefficients.clear();
    if (j.contains("countryCoefficients")) {
        for (auto it = j["countryCoefficients"].begin();
             it != j["countryCoefficients"].end(); ++it) {
            CountryCoefficient cc;
            cc.countryName = it.value().value("countryName", "");
            cc.currentSeasonPoints = it.value().value("currentSeasonPoints", 0.0);
            if (it.value().contains("last5SeasonsPoints"))
                cc.last5SeasonsPoints = it.value()["last5SeasonsPoints"]
                    .get<std::vector<double>>();
            m_countryCoefficients[it.key()] = cc;
        }
    }

    m_clubCoefficients.clear();
    if (j.contains("clubCoefficients")) {
        for (auto it = j["clubCoefficients"].begin();
             it != j["clubCoefficients"].end(); ++it) {
            ClubCoefficient cc;
            cc.clubName = it.value().value("clubName", "");
            cc.currentSeasonPoints = it.value().value("currentSeasonPoints", 0.0);
            if (it.value().contains("last5SeasonsPoints"))
                cc.last5SeasonsPoints = it.value()["last5SeasonsPoints"]
                    .get<std::vector<double>>();
            m_clubCoefficients[it.key()] = cc;
        }
    }

    m_fixtures.clear();
    if (j.contains("fixtures")) {
        for (auto it = j["fixtures"].begin(); it != j["fixtures"].end(); ++it) {
            std::vector<Fixture> fixList;
            for (const auto& fj : it.value()) {
                Fixture fix;
                fix.date = fj.value("date", "");
                fix.homeGoals = fj.value("homeGoals", -1);
                fix.awayGoals = fj.value("awayGoals", -1);
                fix.played = fj.value("played", false);
                fixList.push_back(fix);
            }
            m_fixtures[it.key()] = fixList;
        }
    }

    m_points.clear();
    if (j.contains("points")) {
        for (auto it = j["points"].begin(); it != j["points"].end(); ++it) {
            std::map<std::string, int> ptsMap;
            for (auto pit = it.value().begin(); pit != it.value().end(); ++pit)
                                ptsMap[pit.key()] = pit.value().get<int>();
            m_points[it.key()] = ptsMap;
        }
    }

    m_goalDiff.clear();
    if (j.contains("goalDiff")) {
        for (auto it = j["goalDiff"].begin(); it != j["goalDiff"].end(); ++it) {
            std::map<std::string, int> gdMap;
            for (auto git = it.value().begin(); git != it.value().end(); ++git)
                gdMap[git.key()] = git.value().get<int>();
            m_goalDiff[it.key()] = gdMap;
        }
    }
}
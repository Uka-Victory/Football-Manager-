// InternationalManager.cpp
#include "InternationalManager.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <set>
#include <sstream>
#include <iomanip>
#include <cmath>

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

// ========== CONSTRUCTOR ==========
InternationalManager::InternationalManager() {
}

// ========== CONFEDERATION MAPPING ==========
Confederation InternationalManager::getConfederation(const std::string& country) const {
    // Hardcoded for the countries in world_data.json / typical football nations
    if (country == "England" || country == "Spain" || country == "Italy" ||
        country == "Germany" || country == "France" || country == "Portugal" ||
        country == "Netherlands" || country == "Belgium" || country == "Scotland" ||
        country == "Russia" || country == "Turkey" || country == "Austria" ||
        country == "Switzerland" || country == "Sweden" || country == "Denmark" ||
        country == "Norway" || country == "Poland" || country == "Czech Republic" ||
        country == "Greece" || country == "Ukraine" || country == "Croatia" ||
        country == "Serbia" || country == "Hungary" || country == "Romania" ||
        country == "Bulgaria" || country == "Slovakia" || country == "Slovenia")
        return Confederation::UEFA;

    if (country == "Brazil" || country == "Argentina" || country == "Uruguay" ||
        country == "Colombia" || country == "Chile" || country == "Peru" ||
        country == "Ecuador" || country == "Paraguay" || country == "Bolivia" ||
        country == "Venezuela")
        return Confederation::CONMEBOL;

    if (country == "USA" || country == "Mexico" || country == "Canada" ||
        country == "Costa Rica" || country == "Jamaica" || country == "Honduras" ||
        country == "Panama" || country == "El Salvador" || country == "Trinidad and Tobago")
        return Confederation::CONCACAF;

    if (country == "Nigeria" || country == "Senegal" || country == "Egypt" ||
        country == "Ghana" || country == "Cameroon" || country == "Ivory Coast" ||
        country == "South Africa" || country == "Morocco" || country == "Algeria" ||
        country == "Tunisia" || country == "Mali" || country == "Burkina Faso" ||
        country == "Guinea" || country == "Zambia" || country == "DR Congo")
        return Confederation::CAF;

    if (country == "Japan" || country == "South Korea" || country == "Australia" ||
        country == "Saudi Arabia" || country == "Iran" || country == "Qatar" ||
        country == "China" || country == "UAE" || country == "Iraq" ||
        country == "Uzbekistan" || country == "Oman" || country == "Jordan")
        return Confederation::AFC;

    if (country == "New Zealand" || country == "Fiji" || country == "Tahiti" ||
        country == "Papua New Guinea" || country == "Solomon Islands")
        return Confederation::OFC;

    return Confederation::UEFA; // default
}

std::vector<std::string> InternationalManager::getCountriesInConfederation(
    Confederation conf) const {
    std::vector<std::string> countries;
    for (const auto& kv : m_nationalTeams) {
        if (kv.second.confederation == conf)
            countries.push_back(kv.first);
    }
    return countries;
}

// ========== NATIONAL TEAM GENERATION ==========
void InternationalManager::generateNationalTeams(
    const std::map<std::string, PlayerPtr>& allPlayers,
    const std::vector<std::string>& countries) {

    m_nationalTeams.clear();
    for (const std::string& country : countries) {
        NationalTeam nt;
        nt.country = country;
        nt.confederation = getConfederation(country);
        nt.playerIds.clear();

        std::vector<PlayerPtr> eligible;
        for (const auto& kv : allPlayers) {
            const auto& p = kv.second;
            if (p && p->getNationality() == country && p->getAge() >= 16 && p->getAge() <= 36) {
                eligible.push_back(p);
            }
        }

        std::sort(eligible.begin(), eligible.end(),
            [](const PlayerPtr& a, const PlayerPtr& b) {
                if (a->getOverall() != b->getOverall())
                    return a->getOverall() > b->getOverall();
                return a->getAvgRating() > b->getAvgRating();
            });

        int gkCount = 0, outfieldCount = 0;
        for (const auto& p : eligible) {
            if (p->getPrimaryPosition() == "GK" && gkCount < 3) {
                nt.playerIds.push_back(p->getUniqueId());
                gkCount++;
            } else if (p->getPrimaryPosition() != "GK" && outfieldCount < 20) {
                nt.playerIds.push_back(p->getUniqueId());
                outfieldCount++;
            }
        }

        if (nt.playerIds.size() >= 11)
            m_nationalTeams[country] = nt;
    }
}

void InternationalManager::selectNationalSquad(
    const std::string& country,
    const std::map<std::string, PlayerPtr>& allPlayers) {
    // same as above but uses current fitness/injury status
    auto it = m_nationalTeams.find(country);
    if (it == m_nationalTeams.end()) return;
    NationalTeam& nt = it->second;
    nt.playerIds.clear();

    std::vector<PlayerPtr> eligible;
    for (const auto& kv : allPlayers) {
        const auto& p = kv.second;
        if (p && p->getNationality() == country && p->getAge() >= 16 && p->getAge() <= 36) {
            if (p->isAvailable())
                eligible.push_back(p);
        }
    }

    std::sort(eligible.begin(), eligible.end(),
        [](const PlayerPtr& a, const PlayerPtr& b) {
            if (a->getOverall() != b->getOverall())
                return a->getOverall() > b->getOverall();
            return a->getAvgRating() > b->getAvgRating();
        });

    int gkCount = 0, outfieldCount = 0;
    for (const auto& p : eligible) {
        if (p->getPrimaryPosition() == "GK" && gkCount < 3) {
            nt.playerIds.push_back(p->getUniqueId());
            gkCount++;
        } else if (p->getPrimaryPosition() != "GK" && outfieldCount < 20) {
            nt.playerIds.push_back(p->getUniqueId());
            outfieldCount++;
        }
    }
}

NationalTeam* InternationalManager::getNationalTeam(const std::string& country) {
    auto it = m_nationalTeams.find(country);
    if (it != m_nationalTeams.end()) return &it->second;
    return nullptr;
}

void InternationalManager::updateWorldRankings() {
    // Simplified: rank NT by average overall of top 11
    for (auto& kv : m_nationalTeams) {
        // We need to compute from the global player registry – called from main loop
    }
}

// ========== QUALIFICATION GROUP GENERATION ==========
void InternationalManager::generateQualifyingGroups(
    InternationalTournament tournament,
    const std::vector<std::string>& countries,
    int numGroups, int teamsPerGroup,
    GameCalendar& calendar) {

    m_qualifyingFixtures[tournament].clear();
    m_qualifyingPoints[tournament].clear();
    m_qualifyingGoalDiff[tournament].clear();

    std::vector<std::string> shuffled = countries;
    std::shuffle(shuffled.begin(), shuffled.end(), Utils::getRng());

    // Distribute into groups
    std::vector<std::vector<std::string>> groups(numGroups);
    for (size_t i = 0; i < shuffled.size(); ++i) {
        groups[i % numGroups].push_back(shuffled[i]);
    }

    // Generate round‑robin fixtures for each group
    std::string matchDate = std::to_string(calendar.getYear()) + "-09-01";
    int matchday = 1;
    std::vector<QualifyingFixture> allFixtures;

    for (int g = 0; g < numGroups; ++g) {
        const auto& groupTeams = groups[g];
        std::string groupName = "Group " + std::string(1, 'A' + g);

        // Round‑robin within the group
        for (size_t i = 0; i < groupTeams.size(); ++i) {
            for (size_t j = i + 1; j < groupTeams.size(); ++j) {
                // Home & away legs
                for (int leg = 0; leg < 2; ++leg) {
                    QualifyingFixture fix;
                    fix.date = matchDate;
                    fix.homeCountry = (leg == 0) ? groupTeams[i] : groupTeams[j];
                    fix.awayCountry = (leg == 0) ? groupTeams[j] : groupTeams[i];
                    fix.group = groupName;
                    fix.matchday = matchday;
                    fix.played = false;
                    allFixtures.push_back(fix);
                    matchDate = addDays(matchDate, 7);
                    matchday++;
                }
            }
        }

        // Initialize points for all teams
        for (const auto& team : groupTeams) {
            m_qualifyingPoints[tournament][team] = 0;
            m_qualifyingGoalDiff[tournament][team] = 0;
        }
    }

    m_qualifyingFixtures[tournament] = allFixtures;
}

// ========== QUALIFYING MATCHDAY PROCESSING ==========
void InternationalManager::processQualifyingMatchday(
    InternationalTournament tournament,
    const std::string& date) {

    auto it = m_qualifyingFixtures.find(tournament);
    if (it == m_qualifyingFixtures.end()) return;

    for (auto& fix : it->second) {
        if (fix.date == date && !fix.played) {
            // Simulate the match using a very simplified engine (here we just
            // generate random score based on team strength).
            // In the real game, you'd call MatchEngine with full rosters.
            fix.homeGoals = Utils::randInt(0, 3);
            fix.awayGoals = Utils::randInt(0, 3);
            fix.played = true;

            // Update qualifying standings
            if (fix.homeGoals > fix.awayGoals) {
                m_qualifyingPoints[tournament][fix.homeCountry] += 3;
            } else if (fix.homeGoals == fix.awayGoals) {
                m_qualifyingPoints[tournament][fix.homeCountry] += 1;
                m_qualifyingPoints[tournament][fix.awayCountry] += 1;
            } else {
                m_qualifyingPoints[tournament][fix.awayCountry] += 3;
            }

            m_qualifyingGoalDiff[tournament][fix.homeCountry] += fix.homeGoals - fix.awayGoals;
            m_qualifyingGoalDiff[tournament][fix.awayCountry] += fix.awayGoals - fix.homeGoals;
        }
    }
}

// ========== GET QUALIFIED COUNTRIES ==========
std::vector<std::string> InternationalManager::getQualifiedCountries(
    InternationalTournament tournament) const {

    if (tournament == InternationalTournament::WorldCup)
        return m_worldCupQualified;
    if (tournament == InternationalTournament::UEFAEuros)
        return m_eurosQualified;
    if (tournament == InternationalTournament::CopaAmerica)
        return m_copaAmericaQualified;
    if (tournament == InternationalTournament::AFCON)
        return m_afconQualified;

    return {};
}

std::vector<std::string> InternationalManager::getGroupWinners(
    InternationalTournament tournament,
    const std::string& groupPrefix, int numGroups) const {

    std::vector<std::string> winners;
    auto ptsIt = m_qualifyingPoints.find(tournament);
    if (ptsIt == m_qualifyingPoints.end()) return winners;

    auto gdIt = m_qualifyingGoalDiff.find(tournament);

    for (int g = 0; g < numGroups; ++g) {
        std::string groupName = groupPrefix + std::string(1, 'A' + g);
        // Collect teams in this group
        std::vector<std::pair<std::string, int>> teams;
        for (const auto& kv : ptsIt->second) {
            // Check if this team is in this group (we need the group from fixtures)
            // Simple approach: iterate fixtures to find group membership
            bool inGroup = false;
            auto fixIt = m_qualifyingFixtures.find(tournament);
            if (fixIt != m_qualifyingFixtures.end()) {
                for (const auto& fix : fixIt->second) {
                    if (fix.group == groupName &&
                        (fix.homeCountry == kv.first || fix.awayCountry == kv.first)) {
                        inGroup = true;
                        break;
                    }
                }
            }
            if (inGroup) {
                teams.push_back({kv.first, kv.second});
            }
        }

        // Sort by points, then goal difference
        std::sort(teams.begin(), teams.end(),
            [&](const std::pair<std::string,int>& a, const std::pair<std::string,int>& b) {
                if (a.second != b.second) return a.second > b.second;
                int gdA = 0, gdB = 0;
                if (gdIt != m_qualifyingGoalDiff.end()) {
                    auto itA = gdIt->second.find(a.first);
                    auto itB = gdIt->second.find(b.first);
                    if (itA != gdIt->second.end()) gdA = itA->second;
                    if (itB != gdIt->second.end()) gdB = itB->second;
                }
                return gdA > gdB;
            });

        if (!teams.empty()) winners.push_back(teams[0].first);
    }
    return winners;
}

std::vector<std::string> InternationalManager::getGroupRunnersUp(
    InternationalTournament tournament,
    const std::string& groupPrefix, int numGroups) const {

    std::vector<std::string> runnersUp;
    auto ptsIt = m_qualifyingPoints.find(tournament);
    if (ptsIt == m_qualifyingPoints.end()) return runnersUp;

    auto gdIt = m_qualifyingGoalDiff.find(tournament);

    for (int g = 0; g < numGroups; ++g) {
        std::string groupName = groupPrefix + std::string(1, 'A' + g);
        std::vector<std::pair<std::string, int>> teams;
        for (const auto& kv : ptsIt->second) {
            bool inGroup = false;
            auto fixIt = m_qualifyingFixtures.find(tournament);
            if (fixIt != m_qualifyingFixtures.end()) {
                for (const auto& fix : fixIt->second) {
                    if (fix.group == groupName &&
                        (fix.homeCountry == kv.first || fix.awayCountry == kv.first)) {
                        inGroup = true;
                        break;
                    }
                }
            }
            if (inGroup) teams.push_back({kv.first, kv.second});
        }

        std::sort(teams.begin(), teams.end(),
            [&](const std::pair<std::string,int>& a, const std::pair<std::string,int>& b) {
                if (a.second != b.second) return a.second > b.second;
                int gdA = 0, gdB = 0;
                if (gdIt != m_qualifyingGoalDiff.end()) {
                    auto itA = gdIt->second.find(a.first);
                    auto itB = gdIt->second.find(b.first);
                    if (itA != gdIt->second.end()) gdA = itA->second;
                    if (itB != gdIt->second.end()) gdB = itB->second;
                }
                return gdA > gdB;
            });

        if (teams.size() >= 2) runnersUp.push_back(teams[1].first);
    }
    return runnersUp;
}

// ========== WORLD CUP QUALIFIERS ==========
void InternationalManager::setupWorldCupQualifiers(GameCalendar& calendar) {
    // Set host (simplified: highest‑ranked unused country or a predefined one)
    if (m_worldCupHost.empty()) {
        if (!m_nationalTeams.empty())
            m_worldCupHost = m_nationalTeams.begin()->first;
    }

    // UEFA: 16 slots, 10 groups of ? → 55 teams → 10 groups (5x5, 5x6), top 1 qualifies
    // plus 6 playoff slots for best runners‑up → 3 more.
    // For brevity, we'll do 8 groups of 5, top 1 qualifies (8), plus 8 playoffs for best runners‑up (4),
    // totalling 12. The remaining 4 go via Nations League playoff path (simplified to 4 more runners‑up).
    std::vector<std::string> uefaCountries = getCountriesInConfederation(Confederation::UEFA);
    if (uefaCountries.size() >= 40) {
        generateQualifyingGroups(InternationalTournament::WorldCup, uefaCountries,
                                  8, 5, calendar);
    }

    // CAF: 9 slots, 9 groups, top 1 qualifies
    std::vector<std::string> cafCountries = getCountriesInConfederation(Confederation::CAF);
    if (cafCountries.size() >= 36)
        generateQualifyingGroups(InternationalTournament::WorldCup, cafCountries, 9, 4, calendar);

    // AFC: 8 slots, 8 groups
    std::vector<std::string> afcCountries = getCountriesInConfederation(Confederation::AFC);
    if (afcCountries.size() >= 32)
        generateQualifyingGroups(InternationalTournament::WorldCup, afcCountries, 8, 4, calendar);

    // CONMEBOL: 6 slots, single league of 10 teams (round‑robin)
    std::vector<std::string> conmebolCountries = getCountriesInConfederation(Confederation::CONMEBOL);
    if (conmebolCountries.size() >= 10)
        generateQualifyingGroups(InternationalTournament::WorldCup, conmebolCountries,
                                  1, (int)conmebolCountries.size(), calendar);

    // CONCACAF: 6 slots, 6 groups
    std::vector<std::string> concacafCountries = getCountriesInConfederation(Confederation::CONCACAF);
    if (concacafCountries.size() >= 24)
        generateQualifyingGroups(InternationalTournament::WorldCup, concacafCountries, 6, 4, calendar);

    // OFC: 1 slot, single group
    std::vector<std::string> ofcCountries = getCountriesInConfederation(Confederation::OFC);
    if (ofcCountries.size() >= 4)
        generateQualifyingGroups(InternationalTournament::WorldCup, ofcCountries, 1,
                                  (int)ofcCountries.size(), calendar);

    // After all qualifiers are played (at the end of the cycle), determine the 48 qualified teams.
    // This is called from main loop when the qualifying period ends.
}

void InternationalManager::setupEurosQualifiers(GameCalendar& calendar) {
    if (m_eurosHost.empty() && !m_nationalTeams.empty()) {
        // Pick a UEFA host
        for (const auto& kv : m_nationalTeams) {
            if (kv.second.confederation == Confederation::UEFA) {
                m_eurosHost = kv.first;
                break;
            }
        }
    }

    std::vector<std::string> uefaCountries = getCountriesInConfederation(Confederation::UEFA);
    // Exclude host
    uefaCountries.erase(std::remove(uefaCountries.begin(), uefaCountries.end(), m_eurosHost),
                        uefaCountries.end());

    // 10 groups of 5‑6, top 2 qualify + 3 best 3rd → 23 qualifiers
    if (uefaCountries.size() >= 50)
        generateQualifyingGroups(InternationalTournament::UEFAEuros, uefaCountries,
                                  10, 5, calendar);
}

const std::vector<QualifyingFixture>& InternationalManager::getQualifyingFixtures(
    InternationalTournament tournament) const {

    static std::vector<QualifyingFixture> empty;
    auto it = m_qualifyingFixtures.find(tournament);
    if (it != m_qualifyingFixtures.end()) return it->second;
    return empty;
}

// ========== TOURNAMENT SETUP ==========
void InternationalManager::setupTournament(
    InternationalTournament tournament,
    const std::vector<std::string>& participatingCountries,
    GameCalendar& calendar) {

    m_tournamentFixtures[tournament].clear();
    m_groupPoints[tournament].clear();
    m_groupGoalDiff[tournament].clear();

    size_t numTeams = participatingCountries.size();
    if (numTeams < 4) return;

    size_t numGroups = numTeams / 4;
    std::vector<std::string> shuffled = participatingCountries;
    std::shuffle(shuffled.begin(), shuffled.end(), Utils::getRng());

    std::map<std::string, std::string> countryGroup;
    char groupChar = 'A';
    for (size_t g = 0; g < numGroups; ++g) {
        std::string groupName(1, groupChar + g);
            char groupChar = 'A';
    for (size_t g = 0; g < numGroups; ++g) {
        std::string groupName(1, groupChar + g);
        for (size_t i = 0; i < 4; ++i) {
            std::string country = shuffled[g * 4 + i];
            countryGroup[country] = groupName;
            m_groupPoints[tournament][country] = 0;
            m_groupGoalDiff[tournament][country] = 0;
        }
    }

    std::string matchDate = std::to_string(calendar.getYear()) + "-06-14";
    std::vector<TournamentFixture> groupFixtures;

    for (size_t g = 0; g < numGroups; ++g) {
        std::string groupName(1, groupChar + g);
        std::vector<std::string> groupTeams;
        for (const auto& [country, grp] : countryGroup)
            if (grp == groupName) groupTeams.push_back(country);

        std::vector<std::pair<int,int>> matchups = {{0,1},{2,3},{0,2},{1,3},{0,3},{1,2}};
        for (auto& m : matchups) {
            TournamentFixture fix;
            fix.date = matchDate;
            fix.homeCountry = groupTeams[m.first];
            fix.awayCountry = groupTeams[m.second];
            fix.stage = "Group";
            fix.group = groupName;
            fix.played = false;
            groupFixtures.push_back(fix);
            matchDate = addDays(matchDate, 3);
        }
    }

    m_tournamentFixtures[tournament] = groupFixtures;
}

void InternationalManager::setupWorldCup(GameCalendar& calendar) {
    std::vector<std::string> participants = m_worldCupQualified;
    if (participants.empty()) {
        for (const auto& kv : m_nationalTeams)
            participants.push_back(kv.first);
        std::sort(participants.begin(), participants.end(),
            [this](const std::string& a, const std::string& b) {
                return m_nationalTeams.at(a).worldRanking < m_nationalTeams.at(b).worldRanking;
            });
        if (participants.size() > 48) participants.resize(48);
    }
    setupTournament(InternationalTournament::WorldCup, participants, calendar);
}

void InternationalManager::setupEuros(GameCalendar& calendar) {
    std::vector<std::string> participants = m_eurosQualified;
    if (participants.empty()) {
        for (const auto& kv : m_nationalTeams) {
            if (kv.second.confederation == Confederation::UEFA)
                participants.push_back(kv.first);
        }
        if (participants.size() > 24) participants.resize(24);
    }
    setupTournament(InternationalTournament::UEFAEuros, participants, calendar);
}

void InternationalManager::setupCopaAmerica(GameCalendar& calendar) {
    std::vector<std::string> participants = m_copaAmericaQualified;
    if (participants.empty()) {
        for (const auto& kv : m_nationalTeams) {
            if (kv.second.confederation == Confederation::CONMEBOL)
                participants.push_back(kv.first);
        }
        std::vector<std::string> concacaf = getCountriesInConfederation(Confederation::CONCACAF);
        if (concacaf.size() > 6) concacaf.resize(6);
        participants.insert(participants.end(), concacaf.begin(), concacaf.end());
    }
    setupTournament(InternationalTournament::CopaAmerica, participants, calendar);
}

void InternationalManager::setupAFCON(GameCalendar& calendar) {
    std::vector<std::string> participants = m_afconQualified;
    if (participants.empty()) {
        for (const auto& kv : m_nationalTeams) {
            if (kv.second.confederation == Confederation::CAF)
                participants.push_back(kv.first);
        }
        if (participants.size() > 24) participants.resize(24);
    }
    setupTournament(InternationalTournament::AFCON, participants, calendar);
}

// ========== FIXTURES & RESULTS ==========
const std::vector<TournamentFixture>& InternationalManager::getTournamentFixtures(
    InternationalTournament tournament) const {
    static std::vector<TournamentFixture> empty;
    auto it = m_tournamentFixtures.find(tournament);
    if (it != m_tournamentFixtures.end()) return it->second;
    return empty;
}

void InternationalManager::recordTournamentResult(
    InternationalTournament tournament,
    const std::string& homeCountry,
    const std::string& awayCountry,
    int homeGoals, int awayGoals,
    const std::string& date) {

    auto& fixtures = m_tournamentFixtures[tournament];
    for (auto& fix : fixtures) {
        if (fix.date == date && fix.homeCountry == homeCountry && fix.awayCountry == awayCountry) {
            fix.homeGoals = homeGoals;
            fix.awayGoals = awayGoals;
            fix.played = true;

            if (fix.stage == "Group") {
                int homePts = 0, awayPts = 0;
                if (homeGoals > awayGoals) homePts = 3;
                else if (homeGoals == awayGoals) { homePts = 1; awayPts = 1; }
                else awayPts = 3;

                m_groupPoints[tournament][homeCountry] += homePts;
                m_groupPoints[tournament][awayCountry] += awayPts;
                m_groupGoalDiff[tournament][homeCountry] += homeGoals - awayGoals;
                m_groupGoalDiff[tournament][awayCountry] += awayGoals - homeGoals;
            }
            break;
        }
    }
}

std::map<std::string, int> InternationalManager::getGroupStandings(
    InternationalTournament tournament, const std::string& group) const {

    std::map<std::string, int> standings;
    auto ptsIt = m_groupPoints.find(tournament);
    if (ptsIt == m_groupPoints.end()) return standings;

    auto fixIt = m_tournamentFixtures.find(tournament);
    if (fixIt == m_tournamentFixtures.end()) return standings;

    std::set<std::string> groupCountries;
    for (const auto& fix : fixIt->second) {
        if (fix.group == group) {
            groupCountries.insert(fix.homeCountry);
            groupCountries.insert(fix.awayCountry);
        }
    }

    for (const auto& country : groupCountries) {
        auto p = ptsIt->second.find(country);
        standings[country] = (p != ptsIt->second.end()) ? p->second : 0;
    }
    return standings;
}

// ========== INTERNATIONAL BREAK ==========
bool InternationalManager::isInternationalBreak(const std::string& date) const {
    int year, month, day;
    char dash;
    std::istringstream ss(date);
    ss >> year >> dash >> month >> dash >> day;
    return (month == 3 || month == 6 || month == 9 || month == 10 || month == 11);
}

std::vector<std::string> InternationalManager::getCalledUpPlayers(
    const std::string& country) const {
    auto it = m_nationalTeams.find(country);
    if (it != m_nationalTeams.end()) return it->second.playerIds;
    return {};
}

// ========== YEAR CYCLE ==========
void InternationalManager::setYear(int year) {
    m_currentYear = year;
    m_isWorldCupYear = (year % 4 == 2);
    m_isEurosYear = (year % 4 == 0);
    m_isCopaAmericaYear = (year % 4 == 0);
    m_isAFCONYear = (year % 2 == 1);
}

bool InternationalManager::isTournamentYear(InternationalTournament tournament) const {
    switch (tournament) {
        case InternationalTournament::WorldCup:      return m_isWorldCupYear;
        case InternationalTournament::UEFAEuros:     return m_isEurosYear;
        case InternationalTournament::CopaAmerica:   return m_isCopaAmericaYear;
        case InternationalTournament::AFCON:         return m_isAFCONYear;
        default: return false;
    }
}

// ========== PERSISTENCE ==========
json InternationalManager::toJson() const {
    json j;
    j["currentYear"] = m_currentYear;
    j["worldCupHost"] = m_worldCupHost;
    j["eurosHost"] = m_eurosHost;

    // National teams
    j["nationalTeams"] = json::object();
    for (const auto& kv : m_nationalTeams) {
        json nt;
        nt["country"] = kv.second.country;
        nt["playerIds"] = kv.second.playerIds;
        nt["worldRanking"] = kv.second.worldRanking;
        nt["confederation"] = static_cast<int>(kv.second.confederation);
        j["nationalTeams"][kv.first] = nt;
    }

    // Qualifying fixtures
    auto qualFixToJson = [](const std::vector<QualifyingFixture>& list) {
        json arr = json::array();
        for (const auto& f : list)
            arr.push_back({
                {"date", f.date}, {"homeCountry", f.homeCountry}, {"awayCountry", f.awayCountry},
                {"homeGoals", f.homeGoals}, {"awayGoals", f.awayGoals}, {"played", f.played},
                {"group", f.group}, {"matchday", f.matchday}
            });
        return arr;
    };
    j["qualifyingFixtures"] = json::object();
    for (const auto& kv : m_qualifyingFixtures)
        j["qualifyingFixtures"][std::to_string(static_cast<int>(kv.first))] = qualFixToJson(kv.second);

    // Tournament fixtures
    auto tourFixToJson = [](const std::vector<TournamentFixture>& list) {
        json arr = json::array();
        for (const auto& f : list)
            arr.push_back({
                {"date", f.date}, {"homeCountry", f.homeCountry}, {"awayCountry", f.awayCountry},
                {"homeGoals", f.homeGoals}, {"awayGoals", f.awayGoals}, {"played", f.played},
                {"stage", f.stage}, {"group", f.group}
            });
        return arr;
    };
    j["tournamentFixtures"] = json::object();
    for (const auto& kv : m_tournamentFixtures)
        j["tournamentFixtures"][std::to_string(static_cast<int>(kv.first))] = tourFixToJson(kv.second);

    // Points
    auto ptsToJson = [](const std::map<InternationalTournament, std::map<std::string,int>>& src) {
        json obj = json::object();
        for (const auto& kv : src) {
            json inner = json::object();
            for (const auto& p : kv.second) inner[p.first] = p.second;
            obj[std::to_string(static_cast<int>(kv.first))] = inner;
        }
        return obj;
    };
    j["groupPoints"] = ptsToJson(m_groupPoints);
    j["qualifyingPoints"] = ptsToJson(m_qualifyingPoints);

    // Goal diff
    auto gdToJson = [](const std::map<InternationalTournament, std::map<std::string,int>>& src) {
        json obj = json::object();
        for (const auto& kv : src) {
            json inner = json::object();
            for (const auto& p : kv.second) inner[p.first] = p.second;
            obj[std::to_string(static_cast<int>(kv.first))] = inner;
        }
        return obj;
    };
    j["groupGoalDiff"] = gdToJson(m_groupGoalDiff);
    j["qualifyingGoalDiff"] = gdToJson(m_qualifyingGoalDiff);

    j["worldCupQualified"] = m_worldCupQualified;
    j["eurosQualified"] = m_eurosQualified;
    j["copaAmericaQualified"] = m_copaAmericaQualified;
    j["afconQualified"] = m_afconQualified;

    return j;
}

void InternationalManager::fromJson(const json& j) {
    m_currentYear = j.value("currentYear", 2025);
    setYear(m_currentYear);
    m_worldCupHost = j.value("worldCupHost", "");
    m_eurosHost = j.value("eurosHost", "");

    // National teams
    m_nationalTeams.clear();
    if (j.contains("nationalTeams")) {
        for (auto it = j["nationalTeams"].begin(); it != j["nationalTeams"].end(); ++it) {
            NationalTeam nt;
            nt.country = it.value().value("country", "");
            nt.playerIds = it.value().value("playerIds", std::vector<std::string>{});
            nt.worldRanking = it.value().value("worldRanking", 50);
            nt.confederation = static_cast<Confederation>(it.value().value("confederation", 0));
            m_nationalTeams[it.key()] = nt;
        }
    }

    // Qualifying fixtures
    m_qualifyingFixtures.clear();
    if (j.contains("qualifyingFixtures")) {
        for (auto it = j["qualifyingFixtures"].begin(); it != j["qualifyingFixtures"].end(); ++it) {
            InternationalTournament t = static_cast<InternationalTournament>(std::stoi(it.key()));
            std::vector<QualifyingFixture> list;
            for (const auto& fj : it.value()) {
                QualifyingFixture fix;
                fix.date = fj.value("date", "");
                fix.homeCountry = fj.value("homeCountry", "");
                fix.awayCountry = fj.value("awayCountry", "");
                fix.homeGoals = fj.value("homeGoals", -1);
                fix.awayGoals = fj.value("awayGoals", -1);
                fix.played = fj.value("played", false);
                fix.group = fj.value("group", "");
                fix.matchday = fj.value("matchday", 0);
                list.push_back(fix);
            }
            m_qualifyingFixtures[t] = list;
        }
    }

    // Tournament fixtures
    m_tournamentFixtures.clear();
    if (j.contains("tournamentFixtures")) {
        for (auto it = j["tournamentFixtures"].begin(); it != j["tournamentFixtures"].end(); ++it) {
            InternationalTournament t = static_cast<InternationalTournament>(std::stoi(it.key()));
            std::vector<TournamentFixture> list;
            for (const auto& fj : it.value()) {
                TournamentFixture fix;
                fix.date = fj.value("date", "");
                fix.homeCountry = fj.value("homeCountry", "");
                fix.awayCountry = fj.value("awayCountry", "");
                fix.homeGoals = fj.value("homeGoals", -1);
                fix.awayGoals = fj.value("awayGoals", -1);
                fix.played = fj.value("played", false);
                fix.stage = fj.value("stage", "");
                fix.group = fj.value("group", "");
                list.push_back(fix);
            }
            m_tournamentFixtures[t] = list;
        }
    }

    // Points & GD
    auto loadMap = [](const json& src, std::map<InternationalTournament, std::map<std::string,int>>& dst) {
        dst.clear();
        if (src.is_object()) {
            for (auto it = src.begin(); it != src.end(); ++it) {
                InternationalTournament t = static_cast<InternationalTournament>(std::stoi(it.key()));
                std::map<std::string, int> inner;
                for (auto pit = it.value().begin(); pit != it.value().end(); ++pit)
                    inner[pit.key()] = pit.value().get<int>();
                dst[t] = inner;
            }
        }
    };
    loadMap(j.value("groupPoints", json::object()), m_groupPoints);
    loadMap(j.value("qualifyingPoints", json::object()), m_qualifyingPoints);
    loadMap(j.value("groupGoalDiff", json::object()), m_groupGoalDiff);
    loadMap(j.value("qualifyingGoalDiff", json::object()), m_qualifyingGoalDiff);

    m_worldCupQualified = j.value("worldCupQualified", std::vector<std::string>{});
    m_eurosQualified = j.value("eurosQualified", std::vector<std::string>{});
    m_copaAmericaQualified = j.value("copaAmericaQualified", std::vector<std::string>{});
    m_afconQualified = j.value("afconQualified", std::vector<std::string>{});
}
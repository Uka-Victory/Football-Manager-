// Team.cpp
#include "Team.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <map>
#include <set>

// ========== CONSTRUCTOR ==========
Team::Team(const std::string& name, const std::string& country, int level)
    : m_name(name), m_country(country), m_clubLevel(level) {
    m_stadiumName = name + " Stadium";
    // Generate random colours (simplistic)
    static const std::vector<std::string> colours = {
        "Red", "Blue", "White", "Black", "Yellow", "Green", "Orange", "Purple",
        "Maroon", "Navy", "Sky Blue", "Claret", "Amber", "Crimson", "Gold"
    };
    m_primaryColour   = colours[Utils::randInt(0, colours.size()-1)];
    m_secondaryColour = colours[Utils::randInt(0, colours.size()-1)];
    while (m_secondaryColour == m_primaryColour)
        m_secondaryColour = colours[Utils::randInt(0, colours.size()-1)];
    // Founded year roughly based on level (higher level → older club)
    m_foundedYear = 2025 - Utils::randInt(5, 150);
}

// ========== SQUAD MANAGEMENT ==========
void Team::addToSenior(const PlayerPtr& p) {
    if (!p) return;
    m_seniorSquad.push_back(p);
}

void Team::addToYouth(const PlayerPtr& p) {
    if (!p) return;
    m_youthSquad.push_back(p);
}

void Team::addToAcademy(const PlayerPtr& p) {
    if (!p) return;
    m_academy.push_back(p);
}

bool Team::removePlayer(const std::string& playerId) {
    auto eraseFrom = [&](std::vector<PlayerPtr>& squad) -> bool {
        auto it = std::find_if(squad.begin(), squad.end(),
            [&](const PlayerPtr& p) { return p->getUniqueId() == playerId; });
        if (it != squad.end()) {
            squad.erase(it);
            return true;
        }
        return false;
    };
    if (eraseFrom(m_seniorSquad)) return true;
    if (eraseFrom(m_youthSquad))  return true;
    if (eraseFrom(m_academy))     return true;
    return false;
}

PlayerPtr Team::getPlayerById(const std::string& id) const {
    auto findIn = [&](const std::vector<PlayerPtr>& squad) -> PlayerPtr {
        for (auto& p : squad)
            if (p->getUniqueId() == id) return p;
        return nullptr;
    };
    PlayerPtr found = findIn(m_seniorSquad);
    if (found) return found;
    found = findIn(m_youthSquad);
    if (found) return found;
    return findIn(m_academy);
}

std::vector<PlayerPtr> Team::getAvailablePlayers() const {
    std::vector<PlayerPtr> avail;
    for (auto& p : m_seniorSquad)
        if (p->isAvailable() && p->getFitness() >= 60)
            avail.push_back(p);
    return avail;
}

bool Team::hasEnoughPlayers() const {
    return m_seniorSquad.size() >= 18;
}

bool Team::isOverSquadLimit() const {
    return m_seniorSquad.size() > 35;
}

bool Team::promoteYouthToSenior(const std::string& playerId) {
    auto it = std::find_if(m_youthSquad.begin(), m_youthSquad.end(),
        [&](const PlayerPtr& p) { return p->getUniqueId() == playerId; });
    if (it == m_youthSquad.end()) return false;
    PlayerPtr player = *it;
    m_youthSquad.erase(it);
    m_seniorSquad.push_back(player);
    return true;
}

bool Team::demoteSeniorToYouth(const std::string& playerId) {
    auto it = std::find_if(m_seniorSquad.begin(), m_seniorSquad.end(),
        [&](const PlayerPtr& p) { return p->getUniqueId() == playerId && p->getAge() <= 21; });
    if (it == m_seniorSquad.end()) return false;
    PlayerPtr player = *it;
    m_seniorSquad.erase(it);
    m_youthSquad.push_back(player);
    return true;
}

void Team::processAcademyGraduation(
    const std::vector<std::string>& offeredContracts,
    std::vector<PlayerPtr>& releasedPlayers) {
    std::set<std::string> offeredSet(offeredContracts.begin(), offeredContracts.end());
    std::vector<PlayerPtr> remaining;
    for (auto& p : m_academy) {
        if (offeredSet.count(p->getUniqueId())) {
            m_youthSquad.push_back(p);               // join youth squad
        } else {
            releasedPlayers.push_back(p);             // become free agent
        }
    }
    m_academy.clear();
}

// ========== DEPTH CHART ==========
void Team::rebuildDepthChart() {
    m_depthChart.clear();
    const std::vector<std::string> positions = {
        "GK","CB","LB","RB","DM","CM","LM","RM","AM","LW","RW","ST"
    };
    for (const auto& pos : positions) {
        std::vector<PlayerPtr> eligible;
        for (auto& p : m_seniorSquad) {
            if (p->getPrimaryPosition() == pos || p->getSecondaryPosition() == pos)
                eligible.push_back(p);
        }
        // Sort by overall (descending)
        std::sort(eligible.begin(), eligible.end(),
            [](const PlayerPtr& a, const PlayerPtr& b) {
                return a->getOverall() > b->getOverall();
            });
        std::vector<DepthChartEntry> entries;
        for (size_t i = 0; i < eligible.size(); ++i) {
            entries.push_back({eligible[i]->getUniqueId(), (int)i + 1});
        }
        m_depthChart[pos] = entries;
    }
}

void Team::setDepthChartOrder(const std::string& positionGroup,
                              const std::vector<std::string>& playerIds) {
    std::vector<DepthChartEntry> entries;
    for (size_t i = 0; i < playerIds.size(); ++i) {
        entries.push_back({playerIds[i], (int)i + 1});
    }
    m_depthChart[positionGroup] = entries;
}

std::vector<PlayerPtr> Team::getBestXI(const std::string& formationName) const {
    std::vector<PlayerPtr> xi;
    // Simple 4-3-3 for now (matches slot groups below)
    const std::vector<std::string> slotOrder = {
        "GK",
        "LB","CB","CB","RB",
        "CM","CM","CM",
        "LW","ST","RW"
    };
    std::set<std::string> used;
    for (const auto& slot : slotOrder) {
        PlayerPtr chosen = nullptr;
        auto it = m_depthChart.find(slot);
        if (it != m_depthChart.end()) {
            for (const auto& entry : it->second) {
                PlayerPtr p = getPlayerById(entry.playerId);
                if (p && p->isAvailable() && p->getFitness() >= 60 && !used.count(p->getUniqueId())) {
                    chosen = p;
                    break;
                }
            }
        }
        if (!chosen) {
            // Fallback: pick best available by overall for that position
            int bestOverall = -1;
            for (auto& p : m_seniorSquad) {
                if (p->isAvailable() && p->getFitness() >= 60 && !used.count(p->getUniqueId())) {
                    if (p->getPrimaryPosition() == slot || p->getSecondaryPosition() == slot) {
                        if (p->getOverall() > bestOverall) {
                            bestOverall = p->getOverall();
                            chosen = p;
                        }
                    }
                }
            }
        }
        if (!chosen) {
            // Ultimate fallback: pick any available player
            for (auto& p : m_seniorSquad) {
                if (p->isAvailable() && p->getFitness() >= 60 && !used.count(p->getUniqueId())) {
                    chosen = p;
                    break;
                }
            }
        }
        if (chosen) {
            xi.push_back(chosen);
            used.insert(chosen->getUniqueId());
        }
    }
    return xi;
}

// ========== SEASON RECORD ==========
void Team::recordResult(int gf, int ga) {
    m_gamesPlayed++;
    m_goalsFor += gf;
    m_goalsAgainst += ga;
    if (gf > ga)      { m_wins++;   m_points += 3; }
    else if (gf == ga) { m_draws++;  m_points += 1; }
    else               { m_losses++; }
}

void Team::resetSeasonRecord() {
    m_gamesPlayed = 0; m_wins = 0; m_draws = 0; m_losses = 0;
    m_goalsFor = 0; m_goalsAgainst = 0; m_points = 0;
}

// ========== CLUB RECORDS ==========
void Team::updateRecordsAfterMatch(int ourGoals, int opponentGoals,
                                   const std::string& opponentName) {
    int margin = ourGoals - opponentGoals;
    if (margin > 0 && margin > m_biggestWinMargin) {
        m_biggestWinMargin = margin;
    }
    if (margin < 0 && (-margin) > m_heaviestDefeatMargin) {
        m_heaviestDefeatMargin = -margin;
    }
    if (m_points > m_mostPointsSeason)  m_mostPointsSeason = m_points;
    if (m_goalsFor > m_mostGoalsSeason) m_mostGoalsSeason = m_goalsFor;
}

void Team::updateTransferRecord(int64_t fee, bool isIncome) {
    if (isIncome) {
        if (fee > m_highestTransferReceived) m_highestTransferReceived = fee;
    } else {
        if (fee > m_highestTransferPaid) m_highestTransferPaid = fee;
    }
}

void Team::updatePlayerAgeRecord(int age) {
    if (age < m_youngestPlayerAge) m_youngestPlayerAge = age;
    if (age > m_oldestPlayerAge)   m_oldestPlayerAge = age;
}

void Team::updateMostAppearances(const std::string& playerId) {
    m_mostAppearancesPlayerId = playerId;   // called after recalculateTopLists
}

// ========== TROPHY CABINET ==========
void Team::addTrophy(const std::string& competition, int year, int tier) {
    m_trophyCabinet.push_back({competition, year, tier});
    m_trophyCounts[competition]++;
}

int Team::getTrophyCount(const std::string& competition) const {
    auto it = m_trophyCounts.find(competition);
    return it != m_trophyCounts.end() ? it->second : 0;
}

int Team::getTotalTrophies() const {
    return m_trophyCabinet.size();
}

// ========== ALL‑TIME TOP LISTS ==========
void Team::recalculateTopLists() {
    // Gather all current players (senior + youth) and also all players who ever
    // played for the club. For now we only scan current squads; full history
    // will be added when we have a global player registry with "former club" tracking.
    std::vector<PlayerPtr> allPlayers = m_seniorSquad;
    allPlayers.insert(allPlayers.end(), m_youthSquad.begin(), m_youthSquad.end());

    // Goals
    std::vector<ClubStatLeader> scorers;
    for (auto& p : allPlayers) {
        scorers.push_back({p->getUniqueId(), p->getCareerGoals()});
    }
    std::sort(scorers.begin(), scorers.end(),
        [](const ClubStatLeader& a, const ClubStatLeader& b) { return a.total > b.total; });
    if (scorers.size() > 20) scorers.resize(20);
    m_topScorers = scorers;

    // Assists
    std::vector<ClubStatLeader> assisters;
    for (auto& p : allPlayers) {
        assisters.push_back({p->getUniqueId(), p->getCareerAssists()});
    }
    std::sort(assisters.begin(), assisters.end(),
        [](const ClubStatLeader& a, const ClubStatLeader& b) { return a.total > b.total; });
    if (assisters.size() > 20) assisters.resize(20);
    m_topAssisters = assisters;

    // Appearances
    std::vector<ClubStatLeader> apps;
    for (auto& p : allPlayers) {
        apps.push_back({p->getUniqueId(), p->getCareerApps()});
    }
    std::sort(apps.begin(), apps.end(),
        [](const ClubStatLeader& a, const ClubStatLeader& b) { return a.total > b.total; });
    if (apps.size() > 20) apps.resize(20);
    m_topAppearances = apps;

    // Update most appearances record
    if (!apps.empty()) {
        m_mostAppearancesPlayerId = apps.front().playerId;
    }
}

// ========== CLUB LEGENDS (DYNAMIC THRESHOLDS) ==========
int Team::getLegendAppearanceThreshold() const {
    return (int)(200 * m_clubLevel / 10.0);   // base 200 at level 10
}
int Team::getLegendGoalThreshold(const std::string& position) const {
    double base = 80.0;   // attacker base
    if (position == "CM" || position == "LM" || position == "RM" || position == "DM")
        base = 40.0;
    else if (position == "CB" || position == "LB" || position == "RB")
        base = 20.0;
    else if (position == "GK")
        base = 5.0;
    else
        base = 80.0;  // ST, LW, RW, AM
    return (int)(base * m_clubLevel / 10.0);
}
int Team::getLegendAssistThreshold() const {
    return (int)(60 * m_clubLevel / 10.0);
}
int Team::getLegendCleanSheetThreshold() const {
    return (int)(100 * m_clubLevel / 10.0);
}
int Team::getLegendTrophyThreshold() const {
    return std::max(1, (int)(3 * m_clubLevel / 10.0));
}
int Team::getLegendSeasonThreshold() const {
    return (int)(10 * m_clubLevel / 10.0);
}

bool Team::isLegend(const std::string& playerId) const {
    return std::find(m_clubLegends.begin(), m_clubLegends.end(), playerId)
           != m_clubLegends.end();
}

void Team::checkAndInductLegends(
    const std::map<std::string, PlayerPtr>& globalPlayerRegistry) {
    // Scan all players who belong to this club (current senior + youth)
    std::vector<PlayerPtr> candidates = m_seniorSquad;
    candidates.insert(candidates.end(), m_youthSquad.begin(), m_youthSquad.end());

    // Also scan the global registry for retired players who had this club as homeGrownNation
    // or who were previously in the club (approximate: home nation == club country).
    for (const auto& kv : globalPlayerRegistry) {
        const auto& p = kv.second;
        if (p && p->getHomeGrownNation() == m_country) {
            // crude proxy – could be extended with a proper "club history" vector
            // but to avoid picking up everyone from that country, we require at least
            // some career achievements. For now, we'll rely on current squad.
        }
    }

    for (auto& p : candidates) {
        if (isLegend(p->getUniqueId())) continue;

        int apps    = p->getCareerApps();
        int goals   = p->getCareerGoals();
        int assists = p->getCareerAssists();
        int clean   = p->getCareerCleanSheets();
        int trophies = p->getTrophiesWon();   // we'll need to set this on Player later
        int seasons = p->getAge() - 16;       // approx seasons (will be refined)
        std::string pos = p->getPrimaryPosition();

        bool qualifies = false;
        if (apps    >= getLegendAppearanceThreshold())         qualifies = true;
        if (goals   >= getLegendGoalThreshold(pos))            qualifies = true;
        if (assists >= getLegendAssistThreshold())             qualifies = true;
        if (pos == "GK" && clean >= getLegendCleanSheetThreshold()) qualifies = true;
        if (trophies >= getLegendTrophyThreshold())            qualifies = true;
        if (seasons >= getLegendSeasonThreshold())             qualifies = true;

        // One‑club man: if the player spent their whole career at this club
        // (approximation: homeGrownNation == country && age >= 28)
        if (p->getHomeGrownNation() == m_country && p->getAge() >= 28) {
            qualifies = true;
        }

        // Record breaker: if this player is the club's all‑time top scorer or
        // appearance maker
        if (!m_topScorers.empty() && p->getUniqueId() == m_topScorers[0].playerId)
            qualifies = true;
        if (!m_topAppearances.empty() && p->getUniqueId() == m_topAppearances[0].playerId)
            qualifies = true;

        if (qualifies) {
            m_clubLegends.push_back(p->getUniqueId());
            // In future: generate inbox message
        }
    }
}

void Team::manuallyInductLegend(const std::string& playerId) {
    if (!isLegend(playerId))
        m_clubLegends.push_back(playerId);
}

// ========== HEAD‑TO‑HEAD ==========
HeadToHeadRecord Team::getHeadToHead(const std::string& opponentName) const {
    auto it = m_headToHead.find(opponentName);
    if (it != m_headToHead.end()) return it->second;
    return HeadToHeadRecord{};
}

void Team::updateHeadToHead(const std::string& opponentName,
                            int ourGoals, int opponentGoals,
                            const std::vector<PlayerPtr>& ourPlayers,
                            const std::map<std::string, double>& playerRatings) {
    auto& rec = m_headToHead[opponentName];

    // Update overall record
    if (ourGoals > opponentGoals)       rec.wins++;
    else if (ourGoals == opponentGoals) rec.draws++;
    else                                rec.losses++;

    rec.goalsFor += ourGoals;
    rec.goalsAgainst += opponentGoals;

    int margin = ourGoals - opponentGoals;
    if (margin > 0 && margin > rec.biggestWinMargin)
        rec.biggestWinMargin = margin;
    if (margin < 0 && (-margin) > rec.biggestDefeatMargin)
        rec.biggestDefeatMargin = -margin;

    // Dates (will be set externally from calendar; placeholder)
    // rec.firstMeetingDate / lastMeetingDate should be set by caller

    // Update per‑player stats
    for (auto& p : ourPlayers) {
        auto& stats = rec.playerStats[p->getUniqueId()];
        stats.playerId = p->getUniqueId();
        stats.appearances++;
        // goals/assists are handled separately via match events (MatchEngine calls
        // a dedicated method or the caller passes them). For now, just appearance + rating.
        auto rit = playerRatings.find(p->getUniqueId());
        if (rit != playerRatings.end()) {
            stats.totalRating += rit->second;
        }
    }
}

// ========== RIVALS ==========
bool Team::isRival(const std::string& clubName) const {
    return std::find(m_rivalClubNames.begin(), m_rivalClubNames.end(), clubName)
           != m_rivalClubNames.end();
}

// ========== SERIALISATION ==========
json Team::toJson() const {
    json j;
    j["name"] = m_name;
    j["country"] = m_country;
    j["clubLevel"] = m_clubLevel;
    j["reputation"] = m_reputation;
    j["primaryColour"] = m_primaryColour;
    j["secondaryColour"] = m_secondaryColour;
    j["stadiumName"] = m_stadiumName;
    j["foundedYear"] = m_foundedYear;

    // Squad (store player IDs, full player save/load managed elsewhere)
    j["seniorSquad"] = json::array();
    for (auto& p : m_seniorSquad) j["seniorSquad"].push_back(p->getUniqueId());
    j["youthSquad"] = json::array();
    for (auto& p : m_youthSquad)   j["youthSquad"].push_back(p->getUniqueId());
    j["academy"] = json::array();
    for (auto& p : m_academy)      j["academy"].push_back(p->getUniqueId());

    // Depth chart
    j["depthChart"] = json::object();
    for (auto& kv : m_depthChart) {
        json arr = json::array();
        for (auto& e : kv.second) arr.push_back({{"playerId", e.playerId}, {"rank", e.rank}});
        j["depthChart"][kv.first] = arr;
    }

    // Facilities
    j["facilities"] = m_facilities.toJson();

    // Season record
    j["gamesPlayed"] = m_gamesPlayed; j["wins"] = m_wins;
    j["draws"] = m_draws; j["losses"] = m_losses;
    j["goalsFor"] = m_goalsFor; j["goalsAgainst"] = m_goalsAgainst;
    j["points"] = m_points;

    // Club records
    j["biggestWinMargin"] = m_biggestWinMargin;
    j["heaviestDefeatMargin"] = m_heaviestDefeatMargin;
    j["mostPointsSeason"] = m_mostPointsSeason;
    j["mostGoalsSeason"] = m_mostGoalsSeason;
    j["highestTransferPaid"] = m_highestTransferPaid;
    j["highestTransferReceived"] = m_highestTransferReceived;
    j["youngestPlayerAge"] = m_youngestPlayerAge;
    j["oldestPlayerAge"] = m_oldestPlayerAge;
    j["mostAppearancesPlayerId"] = m_mostAppearancesPlayerId;

    // Trophy cabinet
    j["trophyCabinet"] = json::array();
    for (auto& t : m_trophyCabinet)
        j["trophyCabinet"].push_back({
            {"competition", t.competitionName}, {"year", t.year}, {"tier", t.tier}});
    j["trophyCounts"] = m_trophyCounts;

    // All‑time top lists
    auto leaderToJson = [](const std::vector<ClubStatLeader>& list) {
        json arr = json::array();
        for (auto& l : list) arr.push_back({{"playerId", l.playerId}, {"total", l.total}});
        return arr;
    };
    j["topScorers"]    = leaderToJson(m_topScorers);
    j["topAssisters"]  = leaderToJson(m_topAssisters);
    j["topAppearances"]= leaderToJson(m_topAppearances);

    // Club legends
    j["clubLegends"] = m_clubLegends;

    // Head‑to‑head
    j["headToHead"] = json::object();
    for (auto& kv : m_headToHead) {
        json rec;
        rec["wins"] = kv.second.wins;
        rec["draws"] = kv.second.draws;
        rec["losses"] = kv.second.losses;
        rec["goalsFor"] = kv.second.goalsFor;
        rec["goalsAgainst"] = kv.second.goalsAgainst;
        rec["biggestWinMargin"] = kv.second.biggestWinMargin;
        rec["biggestDefeatMargin"] = kv.second.biggestDefeatMargin;
        rec["firstMeetingDate"] = kv.second.firstMeetingDate;
        rec["lastMeetingDate"] = kv.second.lastMeetingDate;
                rec["playerStats"] = json::object();
                for (auto& ps : kv.second.playerStats) {
                    rec["playerStats"][ps.first] = {
                        {"appearances", ps.second.appearances},
                        {"goals", ps.second.goals},
                        {"assists", ps.second.assists},
                        {"totalRating", ps.second.totalRating}
                    };
                }
                j["headToHead"][kv.first] = rec;
            }

            // Rivals
            j["rivals"] = m_rivalClubNames;

            // Board expectations
            j["boardExpectation"] = m_boardExpectation;
            j["minimumLeaguePosition"] = m_minimumLeaguePosition;

            return j;
        }

        std::shared_ptr<Team> Team::fromJson(const json& j) {
            auto t = std::make_shared<Team>();
            t->m_name            = j.at("name").get<std::string>();
            t->m_country         = j.at("country").get<std::string>();
            t->m_clubLevel       = j.value("clubLevel", 10);
            t->m_reputation      = j.value("reputation", 50);
            t->m_primaryColour   = j.value("primaryColour", "Red");
            t->m_secondaryColour = j.value("secondaryColour", "White");
            t->m_stadiumName     = j.value("stadiumName", t->m_name + " Stadium");
            t->m_foundedYear     = j.value("foundedYear", 1900);

            // Squad IDs (actual Player objects linked later)
            t->m_seniorSquad.clear();
            if (j.contains("seniorSquad") && j["seniorSquad"].is_array())
                for (const auto& id : j["seniorSquad"]) t->m_seniorSquad.push_back(
                    std::make_shared<Player>(id.get<std::string>()));

            t->m_youthSquad.clear();
            if (j.contains("youthSquad") && j["youthSquad"].is_array())
                for (const auto& id : j["youthSquad"]) t->m_youthSquad.push_back(
                    std::make_shared<Player>(id.get<std::string>()));

            t->m_academy.clear();
            if (j.contains("academy") && j["academy"].is_array())
                for (const auto& id : j["academy"]) t->m_academy.push_back(
                    std::make_shared<Player>(id.get<std::string>()));

            // Depth chart
            if (j.contains("depthChart") && j["depthChart"].is_object()) {
                for (auto it = j["depthChart"].begin(); it != j["depthChart"].end(); ++it) {
                    std::vector<DepthChartEntry> entries;
                    for (const auto& e : it.value())
                        entries.push_back({e["playerId"], e["rank"]});
                    t->m_depthChart[it.key()] = entries;
                }
            }

            // Facilities
            if (j.contains("facilities"))
                t->m_facilities = Facilities::fromJson(j["facilities"]);

            // Season record
            t->m_gamesPlayed = j.value("gamesPlayed", 0); t->m_wins = j.value("wins", 0);
            t->m_draws = j.value("draws", 0); t->m_losses = j.value("losses", 0);
            t->m_goalsFor = j.value("goalsFor", 0); t->m_goalsAgainst = j.value("goalsAgainst", 0);
            t->m_points = j.value("points", 0);

            // Club records
            t->m_biggestWinMargin   = j.value("biggestWinMargin", 0);
            t->m_heaviestDefeatMargin = j.value("heaviestDefeatMargin", 0);
            t->m_mostPointsSeason   = j.value("mostPointsSeason", 0);
            t->m_mostGoalsSeason    = j.value("mostGoalsSeason", 0);
            t->m_highestTransferPaid = j.value("highestTransferPaid", 0);
            t->m_highestTransferReceived = j.value("highestTransferReceived", 0);
            t->m_youngestPlayerAge  = j.value("youngestPlayerAge", 99);
            t->m_oldestPlayerAge    = j.value("oldestPlayerAge", 0);
            t->m_mostAppearancesPlayerId = j.value("mostAppearancesPlayerId", "");

            // Trophies
            if (j.contains("trophyCabinet") && j["trophyCabinet"].is_array()) {
                for (const auto& tj : j["trophyCabinet"]) {
                    t->m_trophyCabinet.push_back({
                        tj["competition"].get<std::string>(),
                        tj["year"].get<int>(),
                        tj.value("tier", 0)
                    });
                }
            }
            if (j.contains("trophyCounts"))
                t->m_trophyCounts = j["trophyCounts"].get<std::map<std::string, int>>();

            // All‑time top lists
            auto parseLeader = [](const json& arr, std::vector<ClubStatLeader>& out) {
                out.clear();
                if (arr.is_array()) {
                    for (const auto& e : arr)
                        out.push_back({e["playerId"].get<std::string>(), e["total"].get<int>()});
                }
            };
            if (j.contains("topScorers"))     parseLeader(j["topScorers"], t->m_topScorers);
            if (j.contains("topAssisters"))   parseLeader(j["topAssisters"], t->m_topAssisters);
            if (j.contains("topAppearances")) parseLeader(j["topAppearances"], t->m_topAppearances);

            // Club legends
            if (j.contains("clubLegends") && j["clubLegends"].is_array())
                t->m_clubLegends = j["clubLegends"].get<std::vector<std::string>>();

            // Head‑to‑head
            if (j.contains("headToHead") && j["headToHead"].is_object()) {
                for (auto it = j["headToHead"].begin(); it != j["headToHead"].end(); ++it) {
                    HeadToHeadRecord rec;
                    auto& rj = it.value();
                    rec.wins   = rj.value("wins", 0);
                    rec.draws  = rj.value("draws", 0);
                    rec.losses = rj.value("losses", 0);
                    rec.goalsFor     = rj.value("goalsFor", 0);
                    rec.goalsAgainst = rj.value("goalsAgainst", 0);
                    rec.biggestWinMargin   = rj.value("biggestWinMargin", 0);
                    rec.biggestDefeatMargin = rj.value("biggestDefeatMargin", 0);
                    rec.firstMeetingDate  = rj.value("firstMeetingDate", "");
                    rec.lastMeetingDate   = rj.value("lastMeetingDate", "");
                    if (rj.contains("playerStats")) {
                        for (auto pit = rj["playerStats"].begin(); pit != rj["playerStats"].end(); ++pit) {
                            PlayerH2HStats phs;
                            phs.playerId    = pit.key();
                            phs.appearances = pit.value()["appearances"];
                            phs.goals       = pit.value()["goals"];
                            phs.assists     = pit.value()["assists"];
                            phs.totalRating = pit.value()["totalRating"];
                            rec.playerStats[phs.playerId] = phs;
                        }
                    }
                    t->m_headToHead[it.key()] = rec;
                }
            }

            // Rivals
            if (j.contains("rivals") && j["rivals"].is_array())
                t->m_rivalClubNames = j["rivals"].get<std::vector<std::string>>();

            // Board expectations
            t->m_boardExpectation = j.value("boardExpectation", "Mid‑table finish");
            t->m_minimumLeaguePosition = j.value("minimumLeaguePosition", 12);

            return t;
        }
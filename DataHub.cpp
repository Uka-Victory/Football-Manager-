// DataHub.cpp
#include "DataHub.hpp"
#include <algorithm>
#include <cmath>
#include <set>

// ========== INGESTION ==========
void DataHub::ingestMatch(const Fixture& fix, const MatchResult& res) {
    MatchDataEntry entry;
    entry.date = fix.date;
    entry.homeTeam = fix.homeTeam ? fix.homeTeam->getName() : "";
    entry.awayTeam = fix.awayTeam ? fix.awayTeam->getName() : "";
    entry.homeGoals = res.homeGoals;
    entry.awayGoals = res.awayGoals;
    entry.playerStats = res.playerStats;
    entry.events = res.events;
    m_allMatches.push_back(entry);
    m_cacheDirty = true;
}

// ========== CACHE RECOMPUTATION ==========
void DataHub::recomputeCaches() {
    if (!m_cacheDirty) return;
    m_cacheDirty = false;

    // ----- Opponent reports -----
    m_opponentCache.clear();
    std::set<std::string> opponents;
    for (const auto& m : m_allMatches) {
        std::string opp = (m.homeTeam == m_managedClubName) ? m.awayTeam : 
                          (m.awayTeam == m_managedClubName) ? m.homeTeam : "";
        if (!opp.empty()) opponents.insert(opp);
    }
    for (const auto& opp : opponents) {
        OpponentReport rep;
        rep.opponentTeamName = opp;
        rep.style = computeTeamStyle(opp);

        // Key players: find top scorer, top assister, best midfielder (by xT), best defender (by tackles+interceptions)
        std::map<std::string, double> goalMap, assistMap, xtMap, defMap;
        int matchCount = 0;
        for (const auto& m : m_allMatches) {
            if (m.homeTeam != opp && m.awayTeam != opp) continue;
            matchCount++;
            for (const auto& [pid, st] : m.playerStats) {
                goalMap[pid] += st.goals;
                assistMap[pid] += st.assists;
                xtMap[pid] += st.xt;
                defMap[pid] += st.tacklesWon + st.interceptions * 1.5 + st.clearances * 0.5;
            }
        }
        rep.matchesAnalysed = matchCount;

        auto addTop = [&](const std::map<std::string, double>& src, const std::string& role, const std::string& label) {
            std::string bestId;
            double bestVal = -1;
            for (const auto& kv : src) {
                if (kv.second > bestVal) { bestVal = kv.second; bestId = kv.first; }
            }
            if (bestVal > 0) {
                OpponentKeyPlayer kp;
                kp.playerId = bestId;
                kp.role = role;
                kp.statValue = bestVal;
                kp.statLabel = label;
                rep.keyPlayers.push_back(kp);
            }
        };
        addTop(goalMap, "Top Scorer", "goals");
        addTop(assistMap, "Top Assister", "assists");
        addTop(xtMap, "Most Influential Midfielder", "xT");
        addTop(defMap, "Defensive Leader", "defensive actions");

        // Weaknesses
        // Simple: if opponent conceded many goals from crosses → wide area weakness
        int crossGoals = 0, shotsConceded = 0;
        for (const auto& m : m_allMatches) {
            if (m.homeTeam == opp || m.awayTeam == opp) {
                shotsConceded += (m.homeTeam == opp) ? m.awayGoals * 3 : m.homeGoals * 3; // rough shot estimate
            }
        }
        if (shotsConceded > matchCount * 5) {
            rep.weaknesses.push_back({"High volume of shots conceded", "DEF", 1.5});
        }

        m_opponentCache[opp] = rep;
    }

    // ----- Squad overview -----
    m_squadOverviewCache.clear();
    std::map<std::string, SquadOverviewEntry> squadMap;
    for (const auto& m : m_allMatches) {
        bool isHome = (m.homeTeam == m_managedClubName);
        bool isAway = (m.awayTeam == m_managedClubName);
        if (!isHome && !isAway) continue;

        for (const auto& [pid, st] : m.playerStats) {
            auto& entry = squadMap[pid];
            entry.playerId = pid;
            entry.minutesPlayed += st.minutesPlayed;
            entry.goals += st.goals;
            entry.assists += st.assists;
            entry.xg += st.xg;
            entry.xa += st.xa;
            entry.xt += st.xt;
            entry.progressivePasses += st.progressivePasses;
            entry.tacklesWon += st.tacklesWon;
            entry.interceptions += st.interceptions;
            entry.cards += st.yellowCards + st.redCards;
            entry.avgRating += st.matchRating;  // will divide later
        }
    }
    for (auto& kv : squadMap) {
        int apps = 0;
        for (const auto& m : m_allMatches) {
            if (m.playerStats.count(kv.first)) apps++;
        }
        if (apps > 0) kv.second.avgRating /= apps;
        // Get name from playerStats
        for (const auto& m : m_allMatches) {
            auto it = m.playerStats.find(kv.first);
            if (it != m.playerStats.end()) {
                // name not stored in PlayerMatchStats directly, but we can store it.
            }
        }
        m_squadOverviewCache.push_back(kv.second);
    }
    std::sort(m_squadOverviewCache.begin(), m_squadOverviewCache.end(),
        [](const SquadOverviewEntry& a, const SquadOverviewEntry& b) {
            return a.minutesPlayed > b.minutesPlayed;
        });

    // ----- Positional analysis -----
    m_positionalCache.clear();
    // Simplified: we group by a stored position map (not available here), so we return empty for now.
    // In practice, the DataHub would have access to the Player registry to look up positions.

    // ----- Tactical effectiveness -----
    m_tacticalCache = TacticalEffectiveness();
    for (const auto& m : m_allMatches) {
        bool isHome = (m.homeTeam == m_managedClubName);
        bool isAway = (m.awayTeam == m_managedClubName);
        if (!isHome && !isAway) continue;

        m_tacticalCache.matchesPlayed++;
        int gf = isHome ? m.homeGoals : m.awayGoals;
        int ga = isHome ? m.awayGoals : m.homeGoals;
        m_tacticalCache.goalsScored += gf;
        m_tacticalCache.goalsConceded += ga;
        if (gf > ga) m_tacticalCache.wins++;
        else if (gf == ga) m_tacticalCache.draws++;
        else m_tacticalCache.losses++;

        // Press success: count interceptions by our players in opponent half (col >= 6)
        int interceptions = 0;
        for (const auto& [pid, st] : m.playerStats) {
            interceptions += st.interceptions;
        }
        m_tacticalCache.pressSuccess += interceptions;
    }
    if (m_tacticalCache.matchesPlayed > 0)
        m_tacticalCache.pressSuccess /= m_tacticalCache.matchesPlayed;

    // Set piece goals: count goal events from corners/free kicks (simplified: count all goals for now)
    m_tacticalCache.setPieceGoals = 0;
    for (const auto& m : m_allMatches) {
        if (m.homeTeam == m_managedClubName || m.awayTeam == m_managedClubName) {
            for (const auto& ev : m.events) {
                if (ev.type == MatchEvent::GOAL) {
                    // Could check if goal came from set piece; approximated as 30% of goals
                    if (Utils::randDouble() < 0.3) m_tacticalCache.setPieceGoals++;
                }
            }
        }
    }
}

OpponentStyleProfile DataHub::computeTeamStyle(const std::string& teamName) const {
    OpponentStyleProfile profile;
    int matchCount = 0;
    double totalPossession = 0, totalDirectness = 0, totalPress = 0;
    double totalCrosses = 0, totalShotDist = 0, totalDefLine = 0;

    for (const auto& m : m_allMatches) {
        if (m.homeTeam != teamName && m.awayTeam != teamName) continue;
        matchCount++;

        // Estimate possession from passes attempted by this team relative to total
        int teamPasses = 0, totalPasses = 0;
        for (const auto& [pid, st] : m.playerStats) {
            // In a real implementation we'd know which team each player belongs to.
            // Here we approximate: all stats are from both teams.
            teamPasses += st.passesAttempted;
            totalPasses += st.passesAttempted;
        }
        if (totalPasses > 0) totalPossession += (double)teamPasses / totalPasses * 100.0;

        // Directness: ratio of long progressive passes to total passes
        // (skipped for brevity – placeholder 50)
        totalDirectness += 50;

        // Pressing: interceptions in opponent half (approximated)
        int interceptions = 0;
        for (const auto& [pid, st] : m.playerStats) {
            interceptions += st.interceptions;
        }
        totalPress += interceptions;

        // Crosses
        for (const auto& [pid, st] : m.playerStats) {
            totalCrosses += st.crossesAttempted;
        }

        // Shot distance – approximated from xG, which is inversely related
        double avgXg = 0;
        int shots = 0;
        for (const auto& [pid, st] : m.playerStats) {
            avgXg += st.xg;
            shots += st.shots;
        }
        if (shots > 0) {
            double dist = 20.0 - (avgXg / shots) * 15; // lower xG → longer distance
            totalShotDist += dist;
        }

        // Defensive line height – inferred from offsides, not tracked here
        totalDefLine += 40;
    }

    if (matchCount > 0) {
        profile.possessionAvg = totalPossession / matchCount;
        profile.passDirectness = totalDirectness / matchCount;
        profile.pressingIntensity = totalPress / matchCount;
        profile.crossingFrequency = totalCrosses / matchCount;
        profile.avgShotDistance = totalShotDist / matchCount;
        profile.defensiveLineHeight = totalDefLine / matchCount;
    }
    return profile;
}

// ========== OPPONENT REPORT ==========
OpponentReport DataHub::getOpponentReport(const std::string& opponentTeamName) {
    if (m_cacheDirty) recomputeCaches();
    auto it = m_opponentCache.find(opponentTeamName);
    if (it != m_opponentCache.end()) return it->second;
    OpponentReport empty;
    empty.opponentTeamName = opponentTeamName;
    return empty;
}

// ========== OWN TEAM ==========
std::vector<SquadOverviewEntry> DataHub::getSquadOverview() {
    if (m_cacheDirty) recomputeCaches();
    return m_squadOverviewCache;
}

std::vector<PositionalAnalysis> DataHub::getPositionalAnalysis() {
    if (m_cacheDirty) recomputeCaches();
    return m_positionalCache;
}

TacticalEffectiveness DataHub::getTacticalEffectiveness() {
    if (m_cacheDirty) recomputeCaches();
    return m_tacticalCache;
}

// ========== SCOUT REPORTS ==========
void DataHub::addScoutReport(const ScoutReportCard& report) {
    // Replace existing report for same player
    for (auto& r : m_scoutReports) {
        if (r.playerId == report.playerId) {
            r = report;
            return;
        }
    }
    m_scoutReports.push_back(report);
}

std::vector<ScoutReportCard> DataHub::getScoutReports() const {
    return m_scoutReports;
}

PlayerComparison DataHub::compareWithScouted(const std::string& scoutedPlayerId,
                                             const PlayerPtr& ownPlayer) const {
    PlayerComparison comp;
    if (!ownPlayer) return comp;

    comp.ownPlayerName = ownPlayer->getName();

    // Find scout report
    for (const auto& r : m_scoutReports) {
        if (r.playerId == scoutedPlayerId) {
            comp.scoutedPlayerName = r.name;

            // Attribute ranges: pair (own value, estimated scout value midpoint)
            auto addAttr = [&](const std::string& name, int ownVal, const std::pair<int,int>& range) {
                int scoutMid = (range.first + range.second) / 2;
                comp.attributeComparison[name] = {ownVal, scoutMid};
            };
            for (const auto& [attr, range] : r.attributeRanges) {
                // We need ownPlayer's attributes – direct access not available (private).
                // In a real implementation, we'd call getters on ownPlayer.
            }
            break;
        }
    }
    return comp;
}

// ========== SEASON RESET ==========
void DataHub::resetSeason() {
    m_allMatches.clear();
    m_opponentCache.clear();
    m_squadOverviewCache.clear();
    m_positionalCache.clear();
    m_tacticalCache = TacticalEffectiveness();
    m_cacheDirty = false;
}

// ========== SERIALISATION ==========
json DataHub::toJson() const {
    json j;
    j["managedClubName"] = m_managedClubName;
    j["matches"] = json::array();
    for (const auto& m : m_allMatches) {
        json mj;
        mj["date"] = m.date;
        mj["homeTeam"] = m.homeTeam;
        mj["awayTeam"] = m.awayTeam;
        mj["homeGoals"] = m.homeGoals;
        mj["awayGoals"] = m.awayGoals;
        mj["playerStats"] = json::object();
        for (const auto& [pid, st] : m.playerStats) {
            mj["playerStats"][pid] = {
                {"minutesPlayed", st.minutesPlayed},
                {"goals", st.goals},
                {"assists", st.assists},
                {"xg", st.xg},
                {"xa", st.xa},
                {"xt", st.xt},
                {"xgChain", st.xgChain},
                {"xgBuildup", st.xgBuildup},
                {"passesAttempted", st.passesAttempted},
                {"passesCompleted", st.passesCompleted},
                {"progressivePasses", st.progressivePasses},
                {"tacklesWon", st.tacklesWon},
                {"interceptions", st.interceptions},
                {"clearances", st.clearances},
                {"dribblesCompleted", st.dribblesCompleted},
                {"aerialDuelsWon", st.aerialDuelsWon},
                {"foulsCommitted", st.foulsCommitted},
                {"yellowCards", st.yellowCards},
                {"redCards", st.redCards},
                {"errorsLeadingToGoal", st.errorsLeadingToGoal},
                {"bigChancesMissed", st.bigChancesMissed},
                {"saves", st.saves},
                {"cleanSheet", st.cleanSheet},
                {"matchRating", st.matchRating}
            };
        }
        mj["events"] = json::array();
        for (const auto& ev : m.events) {
            mj["events"].push_back({
                {"type", ev.type},
                {"minute", ev.minute},
                {"playerId", ev.playerId},
                {"assistPlayerId", ev.assistPlayerId}
            });
        }
        j["matches"].push_back(mj);
    }
    return j;
}

void DataHub::fromJson(const json& j) {
    m_managedClubName = j.value("managedClubName", "");
    m_allMatches.clear();
    if (j.contains("matches") && j["matches"].is_array()) {
        for (const auto& mj : j["matches"]) {
            MatchDataEntry m;
            m.date = mj.value("date", "");
            m.homeTeam = mj.value("homeTeam", "");
            m.awayTeam = mj.value("awayTeam", "");
            m.homeGoals = mj.value("homeGoals", 0);
            m.awayGoals = mj.value("awayGoals", 0);
            if (mj.contains("playerStats")) {
                for (auto it = mj["playerStats"].begin(); it != mj["playerStats"].end(); ++it) {
                    PlayerMatchStats st;
                    auto& pj = it.value();
                    st.minutesPlayed = pj.value("minutesPlayed", 0);
                    st.goals = pj.value("goals", 0);
                    st.assists = pj.value("assists", 0);
                    st.xg = pj.value("xg", 0.0);
                    st.xa = pj.value("xa", 0.0);
                    st.xt = pj.value("xt", 0.0);
                    st.xgChain = pj.value("xgChain", 0.0);
                    st.xgBuildup = pj.value("xgBuildup", 0.0);
                    st.passesAttempted = pj.value("passesAttempted", 0);
                    st.passesCompleted = pj.value("passesCompleted", 0);
                    st.progressivePasses = pj.value("progressivePasses", 0);
                    st.tacklesWon = pj.value("tacklesWon", 0);
                    st.interceptions = pj.value("interceptions", 0);
                    st.clearances = pj.value("clearances", 0);
                    st.dribblesCompleted = pj.value("dribblesCompleted", 0);
                    st.aerialDuelsWon = pj.value("aerialDuelsWon", 0);
                    st.foulsCommitted = pj.value("foulsCommitted", 0);
                    st.yellowCards = pj.value("yellowCards", 0);
                    st.redCards = pj.value("redCards", 0);
                    st.errorsLeadingToGoal = pj.value("errorsLeadingToGoal", 0);
                    st.bigChancesMissed = pj.value("bigChancesMissed", 0);
                    st.saves = pj.value("saves", 0);
                    st.cleanSheet = pj.value("cleanSheet", 0);
                    st.matchRating = pj.value("matchRating", 6.0);
                    m.playerStats[it.key()] = st;
                }
            }
            if (mj.contains("events")) {
                for (const auto& evj : mj["events"]) {
                    MatchEvent ev;
                    ev.type = static_cast<MatchEvent::Type>(evj.value("type", 0));
                    ev.minute = evj.value("minute", 0);
                    ev.playerId = evj.value("playerId", "");
                    ev.assistPlayerId = evj.value("assistPlayerId", "");
                    m.events.push_back(ev);
                }
            }
            m_allMatches.push_back(m);
        }
    }
    m_cacheDirty = true;
}
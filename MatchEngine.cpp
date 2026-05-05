// MatchEngine.cpp
#include "MatchEngine.hpp"
#include "Utils.hpp"
#include "Constants.hpp"
#include <algorithm>
#include <cmath>
#include <set>

// ======================== INTERNAL CONSTANTS ========================
// 12 columns × 8 rows pitch grid
// Pre‑computed xT values (expected threat)
static const double xTGrid[12][8] = {
    {0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001},
    {0.002,0.003,0.004,0.005,0.005,0.004,0.003,0.002},
    {0.004,0.007,0.010,0.012,0.012,0.010,0.007,0.004},
    {0.007,0.012,0.018,0.022,0.022,0.018,0.012,0.007},
    {0.010,0.018,0.027,0.035,0.035,0.027,0.018,0.010},
    {0.015,0.027,0.042,0.055,0.055,0.042,0.027,0.015},
    {0.022,0.040,0.065,0.085,0.085,0.065,0.040,0.022},
    {0.035,0.065,0.105,0.140,0.140,0.105,0.065,0.035},
    {0.060,0.110,0.180,0.240,0.240,0.180,0.110,0.060},
    {0.100,0.190,0.310,0.420,0.420,0.310,0.190,0.100},
    {0.180,0.340,0.550,0.700,0.700,0.550,0.340,0.180},
    {0.320,0.600,0.800,0.950,0.950,0.800,0.600,0.320}
};

// Base xG for a shot from a given column (adjusted by pressure)
static double baseXg(int col) {
    if (col < 8) return 0.03;
    if (col == 8) return 0.06;
    if (col == 9) return 0.12;
    if (col == 10) return 0.28;
    return 0.45;  // column 11
}

// ======================== HELPERS ========================
static int getBaseColumn(const std::string& pos) {
    if (pos == "GK") return 0;
    if (pos == "CB" || pos == "LB" || pos == "RB") return 2;
    if (pos == "DM") return 4;
    if (pos == "CM" || pos == "LM" || pos == "RM") return 5;
    if (pos == "AM") return 7;
    if (pos == "LW" || pos == "RW" || pos == "ST") return 8;
    return 5;
}

double MatchEngine::zoneThreat(int col, int row) {
    col = std::max(0, std::min(11, col));
    row = std::max(0, std::min(7, row));
    return xTGrid[col][row];
}

double MatchEngine::shotXg(int col, int row, double pressure) {
    double base = baseXg(col);
    double factor = 1.0 / (1.0 + pressure * 0.1);
    return std::min(0.95, base * factor);
}

// ======================== TEAM STRENGTH ========================
double MatchEngine::computeAttackStrength(const std::vector<PlayerPtr>& xi) {
    double sum = 0.0;
    for (auto& p : xi) {
        double s = (p->getShooting() * 0.25 + p->getDribbling() * 0.15 +
                    p->getPassing() * 0.15 + p->getComposure() * 0.15 +
                    p->getVision() * 0.10 + p->getPace() * 0.10 +
                    p->getAcceleration() * 0.05 + p->getDecisions() * 0.05);
        if (p->getPlaystyle() == Playstyle::Poacher ||
            p->getPlaystyle() == Playstyle::InsideForward)
            s *= 1.10;
        if (p->getPlaystyle() == Playstyle::TargetMan)
            s *= 0.95;
        sum += s;
    }
    return sum / xi.size();
}

double MatchEngine::computeDefenceStrength(const std::vector<PlayerPtr>& xi) {
    double sum = 0.0;
    for (auto& p : xi) {
        double s = (p->getTackling() * 0.25 + p->getPositioning() * 0.25 +
                    p->getStrength() * 0.15 + p->getDecisions() * 0.10 +
                    p->getComposure() * 0.10 + p->getWorkRate() * 0.10 +
                    p->getPace() * 0.05);
        if (p->getPlaystyle() == Playstyle::AnchorMan ||
            p->getPlaystyle() == Playstyle::Stopper)
            s *= 1.10;
        if (p->getPlaystyle() == Playstyle::SweeperKeeper)
            s *= 1.05;
        sum += s;
    }
    return sum / xi.size();
}

double MatchEngine::computeMidfieldControl(const std::vector<PlayerPtr>& xi) {
    double sum = 0.0;
    for (auto& p : xi) {
        double s = (p->getPassing() * 0.20 + p->getVision() * 0.20 +
                    p->getDecisions() * 0.15 + p->getWorkRate() * 0.15 +
                    p->getPositioning() * 0.10 + p->getComposure() * 0.10 +
                    p->getDribbling() * 0.05 + p->getStamina() * 0.05);
        if (p->getPlaystyle() == Playstyle::DeepLyingPlaymaker ||
            p->getPlaystyle() == Playstyle::AdvancedPlaymaker)
            s *= 1.15;
        if (p->getPlaystyle() == Playstyle::BoxToBox)
            s *= 1.05;
        sum += s;
    }
    return sum / xi.size();
}

// ======================== SIMULATE MATCH ========================
MatchResult MatchEngine::simulateMatch(const TeamPtr& home, const TeamPtr& away) {
    MatchResult res;
    auto homeXI = home->getBestXI();
    auto awayXI = away->getBestXI();

    if (homeXI.size() < 11 || awayXI.size() < 11) {
        if (homeXI.size() < 11) { res.homeGoals = 0; res.awayGoals = 3; }
        else { res.homeGoals = 3; res.awayGoals = 0; }
        return res;
    }

    auto initStats = [&](const std::vector<PlayerPtr>& xi) {
        for (auto& p : xi) {
            PlayerMatchStats st;
            st.minutesPlayed = 90;
            res.playerStats[p->getUniqueId()] = st;
        }
    };
    initStats(homeXI);
    initStats(awayXI);

    double homeAtt = computeAttackStrength(homeXI);
    double homeDef = computeDefenceStrength(homeXI);
    double homeMid = computeMidfieldControl(homeXI);
    double awayAtt = computeAttackStrength(awayXI);
    double awayDef = computeDefenceStrength(awayXI);
    double awayMid = computeMidfieldControl(awayXI);

    homeAtt *= 1.05;
    homeMid *= 1.05;

    int totalPossessions = 50 + Utils::randInt(-5, 15);
    double homeShare = homeMid / (homeMid + awayMid + 0.01);
    int homePoss = static_cast<int>(totalPossessions * homeShare);
    int awayPoss = totalPossessions - homePoss;

    for (int i = 0; i < homePoss; ++i)
        simulatePossession(homeXI, awayXI, res, true, homeMid, awayMid);
    for (int i = 0; i < awayPoss; ++i)
        simulatePossession(awayXI, homeXI, res, false, awayMid, homeMid);

    res.homeGoals = res.awayGoals = 0;
    for (auto& ev : res.events) {
        if (ev.type == MatchEvent::GOAL) {
            if (std::find_if(homeXI.begin(), homeXI.end(),
                    [&](const PlayerPtr& p) { return p->getUniqueId() == ev.playerId; }) != homeXI.end())
                res.homeGoals++;
            else
                res.awayGoals++;
        }
    }

    for (auto& p : homeXI) {
        auto& st = res.playerStats[p->getUniqueId()];
        st.matchRating = calculateMatchRating(st, p->getPrimaryPosition());
    }
    for (auto& p : awayXI) {
        auto& st = res.playerStats[p->getUniqueId()];
        st.matchRating = calculateMatchRating(st, p->getPrimaryPosition());
    }

    return res;
}

// ======================== APPLY RESULT ========================
void MatchEngine::applyResultToTeams(const TeamPtr& home, const TeamPtr& away,
                                     MatchResult& res, const std::string& date) {
    home->recordResult(res.homeGoals, res.awayGoals);
    away->recordResult(res.awayGoals, res.homeGoals);

    auto updatePlayer = [&](const TeamPtr& team, const std::string& playerId) {
        auto p = team->getPlayerById(playerId);
        if (!p) return;
        p->recordAppearance();
        auto it = res.playerStats.find(playerId);
        if (it != res.playerStats.end())
            p->addMatchRating(it->second.matchRating);
    };

    for (auto& ev : res.events) {
        if (ev.type == MatchEvent::GOAL) {
            updatePlayer(home, ev.playerId);
            updatePlayer(away, ev.playerId);
            auto p = home->getPlayerById(ev.playerId);
            if (!p) p = away->getPlayerById(ev.playerId);
            if (p) p->recordGoal();

            if (!ev.assistPlayerId.empty()) {
                updatePlayer(home, ev.assistPlayerId);
                updatePlayer(away, ev.assistPlayerId);
                auto a = home->getPlayerById(ev.assistPlayerId);
                if (!a) a = away->getPlayerById(ev.assistPlayerId);
                if (a) a->recordAssist();
            }
        } else if (ev.type == MatchEvent::YELLOW_CARD) {
            updatePlayer(home, ev.playerId);
            updatePlayer(away, ev.playerId);
            auto p = home->getPlayerById(ev.playerId);
            if (!p) p = away->getPlayerById(ev.playerId);
            if (p) p->recordYellowCard();
        } else if (ev.type == MatchEvent::RED_CARD) {
            updatePlayer(home, ev.playerId);
            updatePlayer(away, ev.playerId);
            auto p = home->getPlayerById(ev.playerId);
            if (!p) p = away->getPlayerById(ev.playerId);
            if (p) p->recordRedCard();
        }
    }

    if (res.homeGoals == 0)
        for (auto& p : away->getSeniorSquad()) if (p->getPrimaryPosition() == "GK") p->recordCleanSheet();
    if (res.awayGoals == 0)
        for (auto& p : home->getSeniorSquad()) if (p->getPrimaryPosition() == "GK") p->recordCleanSheet();
}

// ======================== PLAYER RATING ========================
double MatchEngine::calculateMatchRating(const PlayerMatchStats& st,
                                         const std::string& position) {
    double rating = 6.0;
    rating += st.goals * 0.7;
    rating += st.assists * 0.35;
    rating += st.xg * 0.3;
    rating += st.chancesCreated * 0.1;
    rating += st.tacklesWon * 0.08;
    rating += st.interceptions * 0.1;
    rating += st.clearances * 0.03;
    rating += st.dribblesCompleted * 0.05;
    rating += st.aerialDuelsWon * 0.02;

    if (st.passesAttempted > 0) {
        double passPct = (double)st.passesCompleted / st.passesAttempted * 100.0;
        rating += (passPct - 75.0) * 0.05;
    }

    if (st.cleanSheet && (position == "GK" || position == "CB" ||
                          position == "LB" || position == "RB"))
        rating += 0.5;

    rating += st.saves * 0.1;

    rating -= st.foulsCommitted * 0.1;
    rating -= st.yellowCards * 0.3;
    rating -= st.redCards * 1.0;
    rating -= st.errorsLeadingToGoal * 0.5;
    rating -= st.bigChancesMissed * 0.2;

    return std::max(1.0, std::min(10.0, rating));
}

// ======================== POSSESSION SIMULATION ========================
void MatchEngine::simulatePossession(const std::vector<PlayerPtr>& attackXI,
                                     const std::vector<PlayerPtr>& defendXI,
                                     MatchResult& res,
                                     bool isHome,
                                     double homeControl, double awayControl) {
    int ballCol = Utils::randInt(0, 2);
    int ballRow = Utils::randInt(1, 6);

    std::set<std::string> involvedPlayers;
    std::string lastPasserId;
    double startingXT = zoneThreat(ballCol, ballRow);

    int actions = 0;
    const int maxActions = 12;

    while (actions < maxActions && ballCol < 11) {
        actions++;

        PlayerPtr attacker = nullptr;
        double bestWeight = -1.0;
        for (auto& p : attackXI) {
            if (!p->isAvailable()) continue;
            int pCol = getBaseColumn(p->getPrimaryPosition());
            int pRow = Utils::randInt(0, 7);
            int dist = std::abs(pCol - ballCol) + std::abs(pRow - ballRow);
            double weight = (20.0 - dist * 1.5) + p->getOverall() / 10.0;
            if (weight > bestWeight) {
                bestWeight = weight;
                attacker = p;
            }
        }
        if (!attacker) break;

        involvedPlayers.insert(attacker->getUniqueId());

        enum Action { PASS, DRIBBLE, CROSS, SHOOT };
        Action action;
        if (ballCol >= 9) {
            int roll = Utils::randInt(1, 100);
            if (roll <= 40) action = SHOOT;
            else if (roll <= 70 && (ballRow <= 1 || ballRow >= 6)) action = CROSS;
            else action = PASS;
        } else if (ballCol >= 6) {
            int roll = Utils::randInt(1, 100);
            if (roll <= 30) action = DRIBBLE;
            else if (roll <= 95) action = PASS;
            else action = SHOOT;
        } else {
            int roll = Utils::randInt(1, 100);
            if (roll <= 20) action = DRIBBLE;
            else action = PASS;
        }

        if (attacker->hasTrait(Trait::ShootsFromDistance) && ballCol >= 7 && ballCol < 9)
            action = SHOOT;
        if (attacker->hasTrait(Trait::TriesKillerBalls) && ballCol >= 6)
            action = PASS;
            // WideWinger prefers crosses over shots or dribbles
        if (attacker->getPlaystyle() == Playstyle::WideWinger && ballCol >= 7 && ballCol < 10) {
        if (action == SHOOT && Utils::randInt(1,100) <= 40)
        action = CROSS;
        if (action == DRIBBLE && Utils::randInt(1,100) <= 30)
        action = CROSS;
}

        switch (action) {
            case PASS: {
                PlayerPtr receiver = nullptr;
                for (auto& p : attackXI) {
                    if (p == attacker || !p->isAvailable()) continue;
                    int pCol = getBaseColumn(p->getPrimaryPosition());
                    if (pCol > ballCol) { receiver = p; break; }
                }
                if (!receiver) break;

                int distance = std::abs(getBaseColumn(receiver->getPrimaryPosition()) - ballCol);
                bool progressive = (distance >= 3);
                bool intoFinalThird = (getBaseColumn(receiver->getPrimaryPosition()) >= 10);
                bool throughBall = (intoFinalThird && Utils::randInt(1,100) <= 30);

                double passRating = (attacker->getPassing() * 0.4 + attacker->getVision() * 0.3 +
                                     attacker->getDecisions() * 0.2 + attacker->getComposure() * 0.1);
                if (attacker->getPlaystyle() == Playstyle::DeepLyingPlaymaker ||
                    attacker->getPlaystyle() == Playstyle::AdvancedPlaymaker)
                    passRating *= 1.15;
                if (attacker->hasTrait(Trait::TriesKillerBalls) && throughBall)
                    passRating *= 0.9;

                double defendRating = 0.0;
                for (auto& d : defendXI) {
                    if (std::abs(getBaseColumn(d->getPrimaryPosition()) - ballCol) <= 2)
                        defendRating += d->getPositioning() + d->getTackling();
                }
                defendRating /= std::max(1.0, (double)defendXI.size());

                bool success = (passRating + Utils::randInt(-5, 5)) > (defendRating * 0.7);

                auto& attStats = res.playerStats[attacker->getUniqueId()];
                attStats.passesAttempted++;
                if (success) {
                    attStats.passesCompleted++;
                    if (progressive) attStats.progressivePasses++;
                    if (intoFinalThird) attStats.passesIntoFinalThird++;
                    if (throughBall) attStats.throughBalls++;

                    ballCol = getBaseColumn(receiver->getPrimaryPosition());
                    ballRow = Utils::randInt(1, 6);
                    involvedPlayers.insert(receiver->getUniqueId());
                    lastPasserId = attacker->getUniqueId();

                    double newXT = zoneThreat(ballCol, ballRow);
                    double xtEarned = newXT - startingXT;
                    if (xtEarned > 0) {
                        attStats.xt += xtEarned;
                        startingXT = newXT;
                    }
                } else {
                    bool intercepted = (Utils::randInt(0,1) == 0);
                    for (auto& d : defendXI) {
                        if (std::abs(getBaseColumn(d->getPrimaryPosition()) - ballCol) <= 1) {
                            auto& defStats = res.playerStats[d->getUniqueId()];
                            if (intercepted) defStats.interceptions++;
                            else { defStats.tacklesAttempted++; defStats.tacklesWon++; }
                            defStats.recoveries++;
                            break;
                        }
                    }
                    return;
                }
                break;
            }

            case DRIBBLE: {
                auto& attStats = res.playerStats[attacker->getUniqueId()];
                attStats.dribblesAttempted++;

                double dribbleRating = attacker->getDribbling() * 0.4 + attacker->getPace() * 0.2 +
                                       attacker->getAgility() * 0.2 + attacker->getAcceleration() * 0.2;
                if (attacker->getPlaystyle() == Playstyle::InsideForward)
                    dribbleRating *= 1.10;

                double defendRating = 0.0;
                for (auto& d : defendXI) {
                    if (std::abs(getBaseColumn(d->getPrimaryPosition()) - ballCol) <= 1)
                        defendRating += d->getTackling() + d->getPositioning();
                }
                defendRating /= std::max(1.0, (double)defendXI.size());

                bool success = (dribbleRating + Utils::randInt(-5, 5)) > (defendRating * 0.8);

                if (success) {
                    attStats.dribblesCompleted++;
                    int advance = 1 + (attacker->getPace() > 15 ? 1 : 0);
                    ballCol = std::min(11, ballCol + advance);
                    if (advance >= 3) attStats.progressiveCarries++;
                } else {
                    for (auto& d : defendXI) {
                        if (std::abs(getBaseColumn(d->getPrimaryPosition()) - ballCol) <= 1) {
                            auto& defStats = res.playerStats[d->getUniqueId()];
                            defStats.tacklesAttempted++;
                            defStats.tacklesWon++;
                            break;
                        }
                    }
                    return;
                }
                break;
            }

            case CROSS: {
                auto& attStats = res.playerStats[attacker->getUniqueId()];
                attStats.crossesAttempted++;

                double crossRating = attacker->getCrossing() * 0.5 + attacker->getPassing() * 0.3 +
                                     attacker->getVision() * 0.2;
                if (attacker->hasTrait(Trait::CurlsBall)) crossRating *= 1.08;
                if (attacker->getPlaystyle() == Playstyle::WideWinger)
    crossRating *= 1.10;   // +10% cross accuracy for touchline huggers

                bool crossed = false;
                for (auto& recv : attackXI) {
                    if (recv == attacker || !recv->isAvailable()) continue;
                    int rCol = getBaseColumn(recv->getPrimaryPosition());
                    if (rCol >= 10) {
                        crossed = true;
                        double aerialAtt = recv->getHeading() + recv->getStrength();
                        double aerialDef = 0.0;
                        for (auto& d : defendXI)
                            if (getBaseColumn(d->getPrimaryPosition()) >= 10)
                                aerialDef += d->getHeading() + d->getStrength();
                        aerialDef /= std::max(1.0, (double)defendXI.size());

                        if (crossRating + Utils::randInt(-5,5) > aerialDef * 0.6) {
                            attStats.crossesCompleted++;
                            ballCol = rCol;
                            ballRow = 3 + Utils::randInt(-1,1);
                            startingXT = zoneThreat(ballCol, ballRow);
                            involvedPlayers.insert(recv->getUniqueId());
                        } else return;
                        break;
                    }
                }
                if (!crossed) return;
                break;
            }

            case SHOOT: {
                auto& attStats = res.playerStats[attacker->getUniqueId()];
                attStats.shots++;

                double shotRating = attacker->getShooting() * 0.5 + attacker->getComposure() * 0.3 +
                                    attacker->getDecisions() * 0.2;
                if (attacker->hasTrait(Trait::FinesseShot)) shotRating *= 1.10;
                if (attacker->hasTrait(Trait::PowerShot)) shotRating *= 1.05;

                double pressure = 0.0;
                for (auto& d : defendXI)
                    if (std::abs(getBaseColumn(d->getPrimaryPosition()) - ballCol) <= 1)
                        pressure += d->getTackling() + d->getPositioning();

                double xg = shotXg(ballCol, ballRow, pressure);
                xg = std::min(0.95, xg * (shotRating / 15.0));
                attStats.xg += xg;

                bool onTarget = (Utils::randDouble() < (0.6 + shotRating * 0.02));
                if (onTarget) attStats.shotsOnTarget++;

                PlayerPtr gk = nullptr;
                for (auto& d : defendXI)
                    if (d->getPrimaryPosition() == "GK") { gk = d; break; }

                double gkRating = 0.0;
                if (gk) gkRating = gk->getPositioning() * 0.4 + gk->getAgility() * 0.3 +
                                   gk->getComposure() * 0.2 + gk->getDecisions() * 0.1;

                bool goal = (onTarget && (Utils::randDouble() < (xg / (xg + gkRating * 0.02))));
                if (goal) {
                    MatchEvent ev;
                    ev.type = MatchEvent::GOAL;
                    ev.minute = Utils::randInt(1, 90);
                    ev.playerId = attacker->getUniqueId();
                    if (!lastPasserId.empty() && Utils::randDouble() < 0.7) {
                        ev.assistPlayerId = lastPasserId;
                        auto& astStats = res.playerStats[lastPasserId];
                        astStats.xa += xg;
                    }
                    res.events.push_back(ev);
                    res.playerGoals[attacker->getUniqueId()]++;
                    attStats.goals++;

                    for (const auto& pid : involvedPlayers) {
                        auto& pStats = res.playerStats[pid];
                        pStats.xgChain += xg;
                        if (pid != attacker->getUniqueId() && pid != lastPasserId)
                            pStats.xgBuildup += xg;
                    }

                    if (!lastPasserId.empty()) {
                        auto& astStats = res.playerStats[lastPasserId];
                        astStats.chancesCreated++;
                        if (xg > 0.3) astStats.bigChancesCreated++;
                        astStats.sca++;
                        astStats.gca++;
                    }
                } else {
                    if (onTarget && gk) {
                        auto& gkStats = res.playerStats[gk->getUniqueId()];
                        gkStats.saves++;
                    }
                    if (xg > 0.3 && !goal) attStats.bigChancesMissed++;
                }
                return;
            }
        }
    }
}
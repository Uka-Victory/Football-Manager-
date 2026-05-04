// ProgressionEngine.cpp
#include "ProgressionEngine.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <map>
#include <cmath>

// ======================== PHASE CHECKS ========================
bool ProgressionEngine::isInGrowthPhase(int age) {
    return age >= 16 && age <= 23;
}

bool ProgressionEngine::isInPeakPhase(int age) {
    return age >= 24 && age <= 29;
}

bool ProgressionEngine::isInDeclinePhase(int age) {
    return age >= 30;
}

// ======================== MAIN MONTHLY TICK ========================
void ProgressionEngine::processMonthlyTick(
    std::vector<PlayerPtr>& activePlayers,
    const std::vector<TeamPtr>& teams,
    int currentMonth, int currentYear) {

    // 1. Pre‑compute mentorship availability per team
    std::map<std::string, bool> teamMentorMap;  // teamName -> hasMentor
    for (const auto& team : teams) {
        teamMentorMap[team->getName()] = hasMentorInTeam(team);
    }

    // 2. Process each player
    for (auto& player : activePlayers) {
        if (!player) continue;

        int age = player->getAge();
        // Age increases every 12 months (here simplified: we assume birthday in July, so after July age+1)
        // Not handled here; age is updated once per season.

        // Find which club this player belongs to
        TeamPtr club = nullptr;
        for (const auto& team : teams) {
            if (team->getPlayerById(player->getUniqueId())) {
                club = team;
                break;
            }
        }

        // Get this month's match stats (for performance-based growth)
        // In a real system we'd have a per‑player monthly stat accumulator.
        // Here we'll simulate some values based on their season stats (conceptual).
        int matchesThisMonth = 0;
        double avgRatingThisMonth = 0.0;
        if (player->getAppearances() > 0) {
            matchesThisMonth = std::min(4, player->getAppearances());  // rough monthly
            avgRatingThisMonth = player->getAvgRating();
        }

        // Performance‑based attribute affinity (reset each month)
        std::map<std::string, double> matchWeights;
        // In a full implementation, we'd accumulate events from the month's matches.
        // For now, we'll use season‑total stats to derive a rough profile.
        if (player->getGoals() > 0) {
            matchWeights["Shooting"] += player->getGoals() * 0.5;
            matchWeights["Composure"] += player->getGoals() * 0.3;
        }
        if (player->getAssists() > 0) {
            matchWeights["Passing"] += player->getAssists() * 0.3;
            matchWeights["Vision"] += player->getAssists() * 0.3;
        }
        // (more detailed stats would come from DataHub)

        bool hasMentor = false;
        if (club) hasMentor = teamMentorMap[club->getName()];

        // 3. Calculate progression score
        double progressionScore = calculateProgressionScore(
            player, club, matchesThisMonth, avgRatingThisMonth, hasMentor);

        // 4. Determine if any attribute changes
        double threshold = 10.0;  // base threshold for a +1 gain
        if (isInDeclinePhase(age)) {
            // Decline: lower threshold because decay is easier to trigger
            threshold = 8.0;
        }

        if (progressionScore >= threshold) {
            int attrIndex = -1;
            if (isInGrowthPhase(age) || isInPeakPhase(age)) {
                attrIndex = selectAttributeForGrowth(player, matchWeights);
                if (attrIndex >= 0) {
                    // Check potential ceiling
                    int currentVal = 0;
                    int ceiling = player->getPotentialCeiling();
                    // Get current value for that attribute
                    switch (attrIndex) {
                        case 0: currentVal = player->getShooting(); break;
                        case 1: currentVal = player->getPassing(); break;
                        case 2: currentVal = player->getCrossing(); break;
                        case 3: currentVal = player->getDribbling(); break;
                        case 4: currentVal = player->getTackling(); break;
                        case 5: currentVal = player->getHeading(); break;
                        case 6: currentVal = player->getPositioning(); break;
                        case 7: currentVal = player->getComposure(); break;
                        case 8: currentVal = player->getVision(); break;
                        case 9: currentVal = player->getWorkRate(); break;
                        case 10: currentVal = player->getLeadership(); break;
                        case 11: currentVal = player->getDecisions(); break;
                        case 12: currentVal = player->getPace(); break;
                        case 13: currentVal = player->getAcceleration(); break;
                        case 14: currentVal = player->getStamina(); break;
                        case 15: currentVal = player->getStrength(); break;
                        case 16: currentVal = player->getAgility(); break;
                        default: break;
                    }
                    if (currentVal < ceiling) {
                        applyAttributeBump(player, attrIndex, 1);
                    }
                }
            } else if (isInDeclinePhase(age)) {
                attrIndex = selectAttributeForDecline(player);
                if (attrIndex >= 0) {
                    applyAttributeBump(player, attrIndex, -1);
                }
            }
        }

        // 5. Mental compensatory growth for veterans (age 30‑34)
        if (age >= 30 && age <= 34) {
            int mentalAttr = Utils::randInt(0, 3); // Composure, Decisions, Positioning, Leadership
            int currentVal = 0;
            switch (mentalAttr) {
                case 0: currentVal = player->getComposure(); break;
                case 1: currentVal = player->getDecisions(); break;
                case 2: currentVal = player->getPositioning(); break;
                case 3: currentVal = player->getLeadership(); break;
            }
            int cap = 20; // mental hard cap
            if (player->hasTrait(Trait::OneClubMan)) cap = 20; // still 20
            if (currentVal < cap && Utils::randInt(1,100) <= 30) {
                applyAttributeBump(player, mentalAttr + 7, 1); // offset to mental index
            }
        }

        // 6. Injury‑induced ceiling reduction (check if player had a severe injury this month)
        // In a full system, we'd have a flag; for now we call with false.
        checkInjuryCeilingReduction(player, false);

        // 7. Late bloomer check (once per season would be enough; here we simplify to always)
        if (age >= 24 && age <= 26 && player->getCareerApps() < 30 &&
            player->getAvgRating() > 7.5) {
            checkLateBloomerCeilingExpansion(player, player->getAvgRating(), player->getCareerApps());
        }
    }
}

// ======================== PROGRESSION SCORE ========================
double ProgressionEngine::calculateProgressionScore(
    const PlayerPtr& player,
    const TeamPtr& club,
    int matchesPlayedThisMonth,
    double avgRatingThisMonth,
    bool hasMentorBonus) {

    int age = player->getAge();
    double score = 0.0;

    // Base score from age curve
    if (isInGrowthPhase(age)) {
        score = 8.0 + (23 - age) * 0.5;  // younger → faster growth
    } else if (isInPeakPhase(age)) {
        score = 5.0;  // slow growth
    } else if (isInDeclinePhase(age)) {
        score = -5.0 - (age - 30) * 0.5; // faster decline with age
    }

    // Match experience bonus
    if (matchesPlayedThisMonth > 0) {
        score += matchesPlayedThisMonth * 2.0;
        score += (avgRatingThisMonth - 6.5) * 3.0;  // high ratings → faster growth
    }

    // Training facility bonus
    if (club) {
        score += club->getFacilities().training * 0.3;
    }

    // Mentorship bonus
    if (hasMentorBonus && age <= 23) {
        score *= 1.15;
    }

    // Injury proneness penalty (makes growth harder if prone)
    score -= (player->getInjuryProneness() - 10) * 0.2;

    // Playstyle influence (slightly adjusts base growth)
    switch (player->getPlaystyle()) {
        case Playstyle::Poacher:
        case Playstyle::InsideForward:
            score *= 1.05; break;
        case Playstyle::DeepLyingPlaymaker:
        case Playstyle::AdvancedPlaymaker:
            score *= 1.03; break;
        case Playstyle::AnchorMan:
        case Playstyle::Stopper:
            score *= 0.97; break; // defenders grow slightly slower
        default: break;
    }

    // Trait modifiers
    if (player->hasTrait(Trait::Consistent)) score *= 1.05;
    if (player->hasTrait(Trait::Inconsistent)) score *= 0.95;
    if (player->hasTrait(Trait::InjuryProne)) score *= 0.9;

    return score;
}

// ======================== ATTRIBUTE SELECTION ========================
int ProgressionEngine::selectAttributeForGrowth(
    const PlayerPtr& player,
    const std::map<std::string, double>& matchWeights) {

    // Build a weighted table of attribute indices (0–16 for visibles)
    // Weights: position baseline + playstyle bias + match performance bias
    std::vector<double> weights(17, 1.0); // default equal

    // Position baseline (from generation weight tables – simplified mapping)
    std::string pos = player->getPrimaryPosition();
    auto addWeight = [&](int idx, double w) { if (idx >=0 && idx<17) weights[idx] += w; };

    // Position profiles (indicative offsets; not the full table but good enough)
    if (pos == "GK") {
        addWeight(6, 5); addWeight(7, 3); addWeight(8, 2); addWeight(11, 2); addWeight(16, 3);
    } else if (pos == "CB") {
        addWeight(4, 5); addWeight(5, 4); addWeight(6, 4); addWeight(7, 2); addWeight(9, 2); addWeight(11, 2); addWeight(15, 3);
    } else if (pos == "LB" || pos == "RB") {
        addWeight(2, 4); addWeight(3, 3); addWeight(4, 3); addWeight(12, 4); addWeight(13, 3); addWeight(14, 3);
    } else if (pos == "DM") {
        addWeight(4, 4); addWeight(6, 3); addWeight(9, 3); addWeight(11, 3); addWeight(14, 2);
    } else if (pos == "CM") {
        addWeight(1, 4); addWeight(8, 3); addWeight(11, 3); addWeight(9, 2);
    } else if (pos == "LM" || pos == "RM") {
        addWeight(2, 4); addWeight(3, 3); addWeight(12, 3); addWeight(13, 3);
    } else if (pos == "AM") {
        addWeight(0, 3); addWeight(1, 3); addWeight(3, 3); addWeight(7, 2); addWeight(8, 3);
    } else if (pos == "LW" || pos == "RW") {
        addWeight(0, 4); addWeight(3, 4); addWeight(12, 3); addWeight(13, 3);
    } else if (pos == "ST") {
        addWeight(0, 6); addWeight(5, 3); addWeight(6, 3); addWeight(7, 3); addWeight(12, 2);
    }

    // Playstyle bias (shift 0.5–1.0 additional weight)
    switch (player->getPlaystyle()) {
        case Playstyle::Poacher:
            addWeight(0, 2); addWeight(6, 1); addWeight(12, 1); break;
        case Playstyle::DeepLyingPlaymaker:
            addWeight(1, 2); addWeight(8, 2); addWeight(11, 1); break;
        case Playstyle::Stopper:
            addWeight(4, 2); addWeight(6, 1); addWeight(15, 1); break;
        case Playstyle::BoxToBox:
            addWeight(9, 1); addWeight(14, 1); break;
        case Playstyle::TargetMan:
            addWeight(5, 2); addWeight(15, 2); break;
        case Playstyle::SweeperKeeper:
            addWeight(7, 2); addWeight(11, 1); break;
        default: break;
    }

    // Match performance weights (if any)
    for (const auto& kw : matchWeights) {
        if (kw.first == "Shooting") addWeight(0, kw.second);
        else if (kw.first == "Passing") addWeight(1, kw.second);
        else if (kw.first == "Crossing") addWeight(2, kw.second);
        else if (kw.first == "Dribbling") addWeight(3, kw.second);
        else if (kw.first == "Tackling") addWeight(4, kw.second);
        else if (kw.first == "Heading") addWeight(5, kw.second);
        else if (kw.first == "Positioning") addWeight(6, kw.second);
        else if (kw.first == "Composure") addWeight(7, kw.second);
        else if (kw.first == "Vision") addWeight(8, kw.second);
        else if (kw.first == "WorkRate") addWeight(9, kw.second);
        else if (kw.first == "Leadership") addWeight(10, kw.second);
        else if (kw.first == "Decisions") addWeight(11, kw.second);
        else if (kw.first == "Pace") addWeight(12, kw.second);
        else if (kw.first == "Acceleration") addWeight(13, kw.second);
        else if (kw.first == "Stamina") addWeight(14, kw.second);
        else if (kw.first == "Strength") addWeight(15, kw.second);
        else if (kw.first == "Agility") addWeight(16, kw.second);
    }

    // Convert weights to int vector and pick randomly
    std::vector<int> intWeights(17);
    for (int i = 0; i < 17; ++i) intWeights[i] = std::max(1, (int)(weights[i] * 10));
    return Utils::weightedRandomIndex(intWeights);
}

int ProgressionEngine::selectAttributeForDecline(const PlayerPtr& player) {
    // Physical attributes decline faster
    std::vector<int> physicalIndices = {12, 13, 14, 15, 16}; // pace, acc, stam, str, agi
    // Technical/mental decline slower
    std::vector<int> otherIndices = {0,1,2,3,4,5,6,7,8,9,10,11};

    // Weight: 70% physical, 30% other
    if (Utils::randInt(1, 100) <= 70) {
        return physicalIndices[Utils::randInt(0, physicalIndices.size()-1)];
    } else {
        return otherIndices[Utils::randInt(0, otherIndices.size()-1)];
    }
}

// ======================== APPLY ATTRIBUTE CHANGE ========================
void ProgressionEngine::applyAttributeBump(PlayerPtr& player, int attrIndex, int delta) {
    if (!player) return;
    // Map index to attribute setter
    switch (attrIndex) {
        case 0: player->setShooting(player->getShooting() + delta); break;
        case 1: player->setPassing(player->getPassing() + delta); break;
        case 2: player->setCrossing(player->getCrossing() + delta); break;
        case 3: player->setDribbling(player->getDribbling() + delta); break;
        case 4: player->setTackling(player->getTackling() + delta); break;
        case 5: player->setHeading(player->getHeading() + delta); break;
        case 6: player->setPositioning(player->getPositioning() + delta); break;
        case 7: player->setComposure(player->getComposure() + delta); break;
        case 8: player->setVision(player->getVision() + delta); break;
        case 9: player->setWorkRate(player->getWorkRate() + delta); break;
        case 10: player->setLeadership(player->getLeadership() + delta); break;
        case 11: player->setDecisions(player->getDecisions() + delta); break;
        case 12: player->setPace(player->getPace() + delta); break;
        case 13: player->setAcceleration(player->getAcceleration() + delta); break;
        case 14: player->setStamina(player->getStamina() + delta); break;
        case 15: player->setStrength(player->getStrength() + delta); break;
        case 16: player->setAgility(player->getAgility() + delta); break;
        default: break;
    }
}

// ======================== CEILING ADJUSTMENTS ========================
void ProgressionEngine::checkInjuryCeilingReduction(PlayerPtr& player, bool severeInjuryThisMonth) {
    if (!severeInjuryThisMonth) return;
    int ceiling = player->getPotentialCeiling();
    if (ceiling <= 1) return;
    double chance = 0.15; // base 15% chance
    if (player->hasTrait(Trait::InjuryProne)) chance *= 2.0;
    if (Utils::randDouble() < chance) {
        player->setPotentialCeiling(std::max(1, ceiling - 1));
    }
}

void ProgressionEngine::checkLateBloomerCeilingExpansion(PlayerPtr& player, double seasonAvgRating, int careerApps) {
    if (careerApps >= 30) return;
    if (seasonAvgRating < 7.5) return;
    if (player->getAge() < 24 || player->getAge() > 26) return;
    double chance = 0.02; // 2% per monthly tick? Actually once per season. Here we call often, so use low chance.
    if (Utils::randDouble() < 0.02) {
        int bump = Utils::randInt(1, 3);
        int newCeiling = std::min(20, player->getPotentialCeiling() + bump);
        if (newCeiling < 20) {
            player->setPotentialCeiling(newCeiling);
        }
    }
}

// ======================== MENTORSHIP ========================
bool ProgressionEngine::hasMentorInTeam(const TeamPtr& team) {
    if (!team) return false;
    for (const auto& p : team->getSeniorSquad()) {
        if (p->getAge() >= 30 && p->getLeadership() >= 15 && p->getComposure() >= 15) {
            return true;
        }
    }
    return false;
}
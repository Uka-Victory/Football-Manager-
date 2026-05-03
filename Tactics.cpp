#include "Tactics.hpp"

Tactics::Tactics() : formationName("4-3-3"), mentality(Mentality::Balanced), passingStyle(PassingStyle::Mixed), tempo(Tempo::Normal), pressing(PressingIntensity::Medium) {
    setFormation("4-3-3");
}

void Tactics::setFormation(const std::string& formation) {
    formationName = formation;
    basePositions.clear();

    // Goalkeeper is permanently anchored at X=1, Y=4/5
    basePositions.push_back({1, 4});

    if (formation == "4-4-2") {
        basePositions.insert(basePositions.end(), {{3,2}, {3,4}, {3,5}, {3,7}}); // DEF Line
        basePositions.insert(basePositions.end(), {{6,2}, {6,4}, {6,5}, {6,7}}); // MID Line
        basePositions.insert(basePositions.end(), {{10,3}, {10,6}});             // ATT Line
    } else if (formation == "4-3-3") {
        basePositions.insert(basePositions.end(), {{3,2}, {3,4}, {3,5}, {3,7}}); // DEF Line
        basePositions.insert(basePositions.end(), {{6,4}, {6,5}, {5,4}});        // MID (1 DM, 2 CM)
        basePositions.insert(basePositions.end(), {{10,2}, {10,4}, {10,7}});     // ATT (LW, ST, RW)
    } else if (formation == "4-2-3-1") {
        basePositions.insert(basePositions.end(), {{3,2}, {3,4}, {3,5}, {3,7}}); // DEF Line
        basePositions.insert(basePositions.end(), {{5,3}, {5,6}});               // DM Line
        basePositions.insert(basePositions.end(), {{8,2}, {8,4}, {8,7}});        // AM Line
        basePositions.insert(basePositions.end(), {{11,4}});                     // Lone ST
    } else if (formation == "3-5-2") {
        basePositions.insert(basePositions.end(), {{3,3}, {3,4}, {3,5}});        // 3 CBs
        basePositions.insert(basePositions.end(), {{6,1}, {6,3}, {6,4}, {6,5}, {6,8}}); // 5 MIDs
        basePositions.insert(basePositions.end(), {{10,3}, {10,6}});             // 2 STs
    } else {
        // Fallback default
        setFormation("4-4-2");
    }
}

float Tactics::getMentalityRiskMultiplier() const {
    switch (mentality) {
        case Mentality::VeryDefensive: return 0.25f;
        case Mentality::Defensive:     return 0.50f;
        case Mentality::Balanced:      return 1.00f;
        case Mentality::Attacking:     return 1.50f;
        case Mentality::VeryAttacking: return 2.00f;
    }
    return 1.0f;
}

float Tactics::getStaminaDrainMultiplier() const {
    // High pressing drastically increases the fatigue drain in the match engine
    switch (pressing) {
        case PressingIntensity::Low:       return 0.8f;
        case PressingIntensity::Medium:    return 1.0f;
        case PressingIntensity::High:      return 1.3f;
        case PressingIntensity::VeryHigh:  return 1.8f;
    }
    return 1.0f;
}
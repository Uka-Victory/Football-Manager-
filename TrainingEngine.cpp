#include "TrainingEngine.hpp"
#include "Utils.hpp"
#include <algorithm>

void TrainingEngine::processDailyTraining(TeamPtr team) {
    if (!team) return;

    // 1. Recover Fitness
    for (auto& p : team->getSeniorSquad()) {
        if (p->injured || p->suspended) continue;
        
        // Base recovery + Infirmary facility bonus
        int recovery = 5 + (team->getFacilities().getInfirmaryLevel() / 2);
        p->fitness = std::min(100, p->fitness + recovery);
        
        // Match stamina recovers slowly back to base fitness level
        if (p->matchStamina < static_cast<float>(p->fitness)) {
            p->matchStamina += 10.0f;
        }
    }

    // 2. Training Growth & Attrition
    for (auto& p : team->getSeniorSquad()) {
        if (!p->isAvailable()) continue;

        // Players under 24 have a chance to grow PA into CA based on Training Facilities
        if (p->age < 24 && p->currentAbility < p->potentialAbility) {
            double growthChance = (team->getFacilities().getTrainingLevel() * 0.5) + (p->professionalism * 0.2);
            if (Utils::randDouble() * 100.0 < growthChance) {
                p->currentAbility++;
                // Re-calculate atomic stats slightly based on new CA
                p->generateAttributes(team->getLevel(), p->primaryPosition);
            }
        }

        // Training injury risk (scales with low fitness and injury proneness)
        if (p->fitness < 60) {
            double injuryChance = (60.0 - p->fitness) * 0.1 + (p->injuryProneness * 0.05);
            if (Utils::randDouble() * 100.0 < injuryChance) {
                int days = Utils::randInt(3, 21);
                p->applyInjury(days);
            }
        }
    }
}
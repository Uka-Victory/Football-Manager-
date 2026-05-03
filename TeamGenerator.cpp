#include "TeamGenerator.hpp"
#include "Utils.hpp"
#include "Constants.hpp"
#include <algorithm>

TeamGenerator::TeamGenerator(NamePool& pool) : namePool(pool) {}

PlayerPtr TeamGenerator::generatePlayer(const std::string& teamCountry, int teamLevel, const std::string& position, int ageMin, int ageMax, double domesticBias) {
    // ENFORCED CONSTRAINT: Maximum domestic bias is 50%
    if (domesticBias > 0.50) {
        domesticBias = 0.50;
    }

    auto p = std::make_shared<Player>();
    p->uniqueId = "P_" + Utils::generateUniqueId();
    
    // The Melting Pot: Domestic vs Foreign Roll
    std::string playerNation = teamCountry;
    if (Utils::randDouble() > domesticBias) {
        playerNation = namePool.getRandomCountry();
    }

    // Assign cultural name and nationality
    p->name = namePool.generateName(playerNation);
    p->nationality = playerNation;
    p->homeGrownNation = teamCountry; // Regardless of nationality, trained in the club's country
    p->age = Utils::randInt(ageMin, ageMax);
    p->primaryPosition = position;
    
    // CA and PA scale from 1 to 200 based on the 1-20 Team Level
    int baseCA = (teamLevel * 7) + 30; 
    p->currentAbility = std::clamp(baseCA + Utils::randInt(-15, 15), FM::MIN_CA_PA, FM::MAX_CA_PA);
    
    if (p->age < 23) {
        p->potentialAbility = std::clamp(p->currentAbility + Utils::randInt(10, 40), p->currentAbility, FM::MAX_CA_PA);
    } else {
        p->potentialAbility = p->currentAbility; 
    }

    // Pass the 1-20 team level into the atomic generator
    p->generateAttributes(teamLevel, position);
    p->generatePlaystyle(position);
    p->generateTraits();
    
    p->contractLengthMonths = 12 + Utils::randInt(0, 36);
    p->wage = static_cast<int64_t>(p->currentAbility) * 1500LL; // Base wage scaling
    
    return p;
}

void TeamGenerator::populateTeam(TeamPtr team, int seniorCount, int youthCount) {
    if (!team) return;
    
    int level = team->getLevel(); // 1-20
    std::string country = team->getCountry();

    // 1. Generate Senior Squad (30 Players, max 50% Domestic)
    int gks = 3, defs = 9, mids = 12; // remaining 6 naturally go to attack
    
    for (int i = 0; i < seniorCount; ++i) {
        std::string pos;
        
        if (i < gks) {
            pos = "GK";
        } else if (i < gks + defs) {
            // Distribute defenders: roughly 50% CB, 25% LB, 25% RB
            double r = Utils::randDouble();
            pos = (r > 0.5) ? "CB" : (r > 0.25 ? "LB" : "RB");
        } else if (i < gks + defs + mids) {
            // Distribute midfielders
            double r = Utils::randDouble();
            pos = (r > 0.6) ? "CM" : (r > 0.4 ? "DM" : (r > 0.2 ? "AM" : (r > 0.1 ? "LM" : "RM")));
        } else {
            // Attackers
            double r = Utils::randDouble();
            pos = (r > 0.4) ? "ST" : (r > 0.2 ? "LW" : "RW");
        }
        
        // Pass 0.50 to enforce the 50% maximum domestic cap
        team->addPlayer(generatePlayer(country, level, pos, 18, 34, 0.50), false);
    }

    // 2. Generate Youth Squad (Regens) (20 Players, max 50% Domestic)
    for (int i = 0; i < youthCount; ++i) {
        std::string pos = Utils::randomPosition();
        team->addPlayer(generatePlayer(country, level, pos, 15, 18, 0.50), true);
    }
}
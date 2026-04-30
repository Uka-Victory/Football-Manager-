#include "TeamGenerator.hpp"
#include "Utils.hpp"
#include <string>

TeamGenerator::TeamGenerator(NamePool& pool) : namePool(pool) {}

PlayerPtr TeamGenerator::generatePlayer(const std::string& teamName, int teamLevel, const std::string& position, const std::string& role, int ageMin, int ageMax) {
    
    int baseOvr;
    switch (teamLevel) {
        case 1: baseOvr = Utils::randInt(75, 88); break; 
        case 2: baseOvr = Utils::randInt(65, 76); break; 
        case 3: baseOvr = Utils::randInt(55, 66); break; 
        default: baseOvr = Utils::randInt(45, 56); break; 
    }

    int age = Utils::randInt(ageMin, ageMax);
    
    int pot = baseOvr;
    if (age < 24) {
        pot = baseOvr + Utils::randInt(5, 15);
    } else if (age < 29) {
        pot = baseOvr + Utils::randInt(1, 5);
    }
    if (pot > 99) pot = 99; 

    // BUG FIXED: We now use a randomized hex string. No more ID collisions!
    std::string uniqueId = Utils::generateUniqueId("P_");
    
    std::string name = namePool.generateName();
    std::string nationality = "English"; 

    auto p = std::make_shared<Player>(uniqueId, name, age, nationality, position, role, baseOvr, pot);

    int64_t baseValue = 100000;
    if (baseOvr >= 85) baseValue = 50000000;
    else if (baseOvr >= 80) baseValue = 25000000;
    else if (baseOvr >= 70) baseValue = 5000000;
    else if (baseOvr >= 60) baseValue = 1000000;
    
    if (age < 23 && pot > baseOvr + 5) {
        baseValue = (int64_t)(baseValue * 1.5);
    }

    p->askingPrice = baseValue + Utils::randInt(-50000, 500000); 
    p->wage = baseValue / 520; 

    return p;
}

void TeamGenerator::populateTeam(TeamPtr team) {
    if (!team) return;

    int level = team->getLevel();
    std::string tName = team->getName();

    int64_t transferB = 0;
    int64_t wageB = 0;
    switch(level) {
        case 1: transferB = 80000000; wageB = 1500000; break;
        case 2: transferB = 15000000; wageB = 300000; break;
        case 3: transferB = 3000000; wageB = 80000; break;
        default: transferB = 500000; wageB = 15000; break;
    }
    
    team->setBudgets(transferB, wageB);
    team->addFunds(transferB / 2); 

    // Minimum 23, Maximum 32
    team->addPlayer(generatePlayer(tName, level, "GK", "Goalkeeper", 20, 34)); 
    for(int i = 0; i < 2; i++) team->addPlayer(generatePlayer(tName, level, "GK", "Backup", 18, 38));

    int numCBs = Utils::randInt(4, 5);
    int numFBs = Utils::randInt(4, 5);
    for(int i = 0; i < numCBs; i++) team->addPlayer(generatePlayer(tName, level, "DEF", "Center Back", 19, 33));
    for(int i = 0; i < numFBs; i++) team->addPlayer(generatePlayer(tName, level, "DEF", "Full Back", 18, 31));

    int numCMs = Utils::randInt(4, 5);
    int numDMs = Utils::randInt(2, 3);
    int numAMs = Utils::randInt(2, 3);
    for(int i = 0; i < numCMs; i++) team->addPlayer(generatePlayer(tName, level, "MID", "Central Mid", 18, 34));
    for(int i = 0; i < numDMs; i++) team->addPlayer(generatePlayer(tName, level, "MID", "Defensive Mid", 20, 34));
    for(int i = 0; i < numAMs; i++) team->addPlayer(generatePlayer(tName, level, "MID", "Attacking Mid", 17, 32));

    int numSTs = Utils::randInt(2, 4);
    int numRWs = Utils::randInt(1, 2);
    int numLWs = Utils::randInt(1, 2);
    for(int i = 0; i < numSTs; i++) team->addPlayer(generatePlayer(tName, level, "FWD", "Striker", 18, 34));
    for(int i = 0; i < numRWs; i++) team->addPlayer(generatePlayer(tName, level, "FWD", "Right Winger", 17, 31));
    for(int i = 0; i < numLWs; i++) team->addPlayer(generatePlayer(tName, level, "FWD", "Left Winger", 17, 31));
}
#ifndef TEAMGENERATOR_HPP
#define TEAMGENERATOR_HPP

#include "Team.hpp"
#include "NamePool.hpp"
#include <vector>

class TeamGenerator {
public:
    TeamGenerator(NamePool& namePool);
    
    // Generate a full team with senior and youth players based on team level
    Team generateTeam(const std::string& teamName, const std::string& country, int teamLevel);
    
private:
    NamePool& namePool;
    
    Player createSeniorPlayer(const std::string& country, int teamLevel, const std::string& position);
    Player createYouthPlayer(const std::string& country, int teamLevel, const std::string& position);
    
    int getOverallForLevel(int level, bool isYouth);
    std::string getRandomPosition();
    int getInitialBudget(int level);
};

#endif
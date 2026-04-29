#ifndef TEAMGENERATOR_HPP
#define TEAMGENERATOR_HPP

#include <string>
#include "Team.hpp"
#include "NamePool.hpp"

class TeamGenerator {
private:
    NamePool& namePool;

    // Internal helper to generate a single player using Smart Pointers
    PlayerPtr generatePlayer(const std::string& teamName, int teamLevel, const std::string& position, const std::string& role, int ageMin, int ageMax);

public:
    // Pass the NamePool by reference for massive performance gains
    TeamGenerator(NamePool& pool);

    // Populates an existing team pointer with 23 players and sets initial budgets
    void populateTeam(TeamPtr team);
};

#endif
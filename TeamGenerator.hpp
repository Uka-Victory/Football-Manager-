#ifndef TEAMGENERATOR_HPP
#define TEAMGENERATOR_HPP

#include <string>
#include "Team.hpp"
#include "NamePool.hpp"
#include "WorldData.hpp"

class TeamGenerator {
private:
    NamePool& namePool;

    PlayerPtr generatePlayer(const std::string& teamCountry, int teamLevel, const std::string& position, int ageMin, int ageMax);

public:
    // Pass the NamePool by reference to utilize the massive JSON dataset efficiently
    TeamGenerator(NamePool& pool);

    // Populates a team using the strict 1-20 scale from the countries_data.json
    void populateTeam(TeamPtr team, int seniorCount = 22, int youthCount = 8);
};

#endif
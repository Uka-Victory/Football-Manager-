// TeamGenerator.hpp
#pragma once
#include "Team.hpp"
#include "NamePool.hpp"
#include "WorldData.hpp"
#include <memory>

class TeamGenerator {
public:
    TeamGenerator(const NamePool& np) : namePool(np) {}
    std::shared_ptr<Team> generateTeam(const TeamInfo& info, int level);
    std::shared_ptr<Player> generateYouthPlayer(const std::string& country, int academyLevel);
private:
    const NamePool& namePool;
};
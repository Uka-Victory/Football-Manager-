#pragma once
#include "Team.hpp"
#include "NamePool.hpp"
#include "Player.hpp"
#include <memory>
#include <string>

namespace FootballManager {

    class TeamGenerator {
    public:
        static std::shared_ptr<Player> generatePlayer(const std::string& teamCountry,
                                                       Position pos,
                                                       int ageMin, int ageMax,
                                                       int baseLevel,
                                                       NamePool& namePool);

        static void populateTeam(std::shared_ptr<Team> team, int squadSize, NamePool& namePool);
    };

} // namespace FootballManager
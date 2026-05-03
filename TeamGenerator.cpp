#include "TeamGenerator.hpp"
#include "Utils.hpp"
#include <algorithm>

namespace FootballManager {

    std::shared_ptr<Player> TeamGenerator::generatePlayer(const std::string& teamCountry,
                                                           Position pos,
                                                           int ageMin, int ageMax,
                                                           int baseLevel,
                                                           NamePool& namePool) {
        // Fix: Use Utils::randInt (seeded once at startup) instead of creating
        // a new std::random_device + std::mt19937 per player call
        std::string id   = "P_" + std::to_string(Utils::randInt(10000, 99999));
        std::string name = namePool.generateName(teamCountry);
        int age          = Utils::randInt(ageMin, ageMax);

        auto player = std::make_shared<Player>(id, name, age, pos);

        // Scale club level (1–100) down to the 1–20 attribute range
        int attrBase = std::clamp(baseLevel / 5, 5, 20);

        player->setAttribute("finishing",   std::clamp(attrBase + Utils::randInt(-2, 2), 1, 20));
        player->setAttribute("passing",     std::clamp(attrBase + Utils::randInt(-2, 2), 1, 20));
        player->setAttribute("tackling",    std::clamp(attrBase + Utils::randInt(-2, 2), 1, 20));
        player->setAttribute("pace",        std::clamp(attrBase + Utils::randInt(-2, 2), 1, 20));
        player->setAttribute("vision",      std::clamp(attrBase + Utils::randInt(-2, 2), 1, 20));
        player->setAttribute("positioning", std::clamp(attrBase + Utils::randInt(-2, 2), 1, 20));

        player->assignContract(attrBase * 1000, 2028);

        return player;
    }

    void TeamGenerator::populateTeam(std::shared_ptr<Team> team, int squadSize, NamePool& namePool) {
        if (!team) return;

        std::string country = namePool.getRandomCountry();
        int baseLevel       = team->getClubLevel();

        // Always generate exactly one GK at index 0
        team->addPlayerToSenior(generatePlayer(country, Position::GK, 18, 34, baseLevel, namePool));

        // Cycle through outfield positions for the rest of the squad
        for (int i = 1; i < squadSize; ++i) {
            Position pos = static_cast<Position>((i % 9) + 1);
            team->addPlayerToSenior(generatePlayer(country, pos, 18, 34, baseLevel, namePool));
        }
    }

} // namespace FootballManager
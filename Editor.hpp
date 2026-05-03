#pragma once
#include "Team.hpp"
#include "Player.hpp"
#include <string>
#include <memory>

namespace FootballManager {

    class Editor {
    public:
        // God-mode overrides for testing and debugging
        static void editPlayerAttribute(std::shared_ptr<Player> player, const std::string& attrName, int newValue);
        static void healPlayer(std::shared_ptr<Player> player);
        static void forceTransfer(std::shared_ptr<Team> buyingTeam, std::shared_ptr<Team> sellingTeam, std::shared_ptr<Player> player);
    };

} // namespace FootballManager
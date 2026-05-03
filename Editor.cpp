#include "Editor.hpp"
#include <iostream>

namespace FootballManager {

    void Editor::editPlayerAttribute(std::shared_ptr<Player> player, const std::string& attrName, int newValue) {
        if (!player) return;
        player->setAttribute(attrName, newValue);
        std::cout << "[EDITOR] " << player->getName() << "'s " << attrName << " set to " << newValue << ".\n";
    }

    void Editor::healPlayer(std::shared_ptr<Player> player) {
        if (!player) return;
        player->setFitness(100);
        std::cout << "[EDITOR] " << player->getName() << " has been fully healed to 100% fitness.\n";
    }

    void Editor::forceTransfer(std::shared_ptr<Team> buyingTeam, std::shared_ptr<Team> sellingTeam, std::shared_ptr<Player> player) {
        if (!buyingTeam || !sellingTeam || !player) return;
        
        // Remove from old team
        sellingTeam->releasePlayer(player);
        
        // Force onto new team
        buyingTeam->addPlayerToSenior(player);
        std::cout << "[EDITOR] Forced transfer of " << player->getName() << " completed successfully.\n";
    }

} // namespace FootballManager
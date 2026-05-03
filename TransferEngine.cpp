#include "TransferEngine.hpp"
#include <iostream>

namespace FootballManager {

    void TransferEngine::processAITransfers(std::vector<std::shared_ptr<Team>>& allTeams) {
        if (allTeams.size() < 2) return;

        // Simplified logic replacing the phantom CA/PA variables
        for (auto& team : allTeams) {
            auto roster = team->getSeniorSquad();
            if (roster.empty()) continue;

            // Simple logic: if squad is too large, release the lowest selection index player
            if (roster.size() > 25) {
                auto playerToRelease = roster.back(); 
                team->releasePlayer(playerToRelease);
                std::cout << "[TRANSFER] " << team->getClubLevel() << " released " << playerToRelease->getName() << "\n";
            }
        }
    }

} // namespace FootballManager
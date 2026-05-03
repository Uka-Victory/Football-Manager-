#include "WorldData.hpp"
#include <algorithm>

namespace FootballManager {

    WorldData::WorldData() {}

    void WorldData::registerPlayer(PlayerPtr player) {
        if (player) {
            globalPlayerRegistry[player->getId()] = player;
        }
    }

    void WorldData::addFreeAgent(PlayerPtr player) {
        if (player) {
            // Ensure they aren't already in the pool
            auto it = std::find(freeAgentPool.begin(), freeAgentPool.end(), player);
            if (it == freeAgentPool.end()) {
                // Clear contract data
                player->assignContract(0, 0); 
                freeAgentPool.push_back(player);
            }
        }
    }

    void WorldData::removeFreeAgent(const std::string& playerId) {
        freeAgentPool.erase(
            std::remove_if(freeAgentPool.begin(), freeAgentPool.end(),
                [&playerId](const PlayerPtr& p) { return p->getId() == playerId; }),
            freeAgentPool.end()
        );
    }

    PlayerPtr WorldData::findPlayerGlobally(const std::string& playerId) const {
        auto it = globalPlayerRegistry.find(playerId);
        if (it != globalPlayerRegistry.end()) {
            return it->second;
        }
        return nullptr;
    }

    void WorldData::addToWatchlist(const std::string& playerId) {
        if (std::find(scoutWatchlist.begin(), scoutWatchlist.end(), playerId) == scoutWatchlist.end()) {
            scoutWatchlist.push_back(playerId);
        }
    }

    void WorldData::removeFromWatchlist(const std::string& playerId) {
        scoutWatchlist.erase(
            std::remove(scoutWatchlist.begin(), scoutWatchlist.end(), playerId),
            scoutWatchlist.end()
        );
    }

    void WorldData::processAprilFirstGraduation(std::vector<std::shared_ptr<Team>>& allWorldTeams, int currentYear) {
        for (auto& team : allWorldTeams) {
            std::vector<PlayerPtr> graduates = team->getGraduatingAcademyPlayers(currentYear);
            
            for (auto& prospect : graduates) {
                // AI Logic for AI Teams: Sign if Potential is high, else release
                // User team logic will be intercepted by main.cpp before this auto-resolves.
                
                // For engine fallback: if they hit this loop without a contract, they are released.
                if (prospect->getWeeklyWage() == 0) {
                    team->releasePlayer(prospect);
                    addFreeAgent(prospect);
                }
            }
        }
    }

    void WorldData::processJuneThirtiethMidnightWipe() {
        // Zero out volatile stats for every player in the world, maintaining the Watchlist
        for (auto& pair : globalPlayerRegistry) {
            if (pair.second) {
                pair.second->processMidnightWipe();
            }
        }
    }

} // namespace FootballManager
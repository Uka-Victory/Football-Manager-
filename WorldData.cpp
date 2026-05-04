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
            auto it = std::find(freeAgentPool.begin(), freeAgentPool.end(), player);
            if (it == freeAgentPool.end()) {
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
        if (it != globalPlayerRegistry.end()) return it->second;
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

    void WorldData::processAprilFirstGraduation(
            std::vector<std::shared_ptr<Team>>& allWorldTeams, int currentYear) {

        for (auto& team : allWorldTeams) {
            // Fix: Team has no getGraduatingAcademyPlayers().
            // Instead, scan the senior squad for players whose academy
            // join year means they've served the 1-year residency rule.
            std::vector<PlayerPtr> toRelease;
            for (const auto& player : team->getSeniorSquad()) {
                int joinYear = player->getAcademyJoinYear();
                // joinYear == 0 means they weren't an academy prospect
                if (joinYear != 0 && (currentYear - joinYear) >= 1
                        && player->getWeeklyWage() == 0) {
                    toRelease.push_back(player);
                }
            }
            for (auto& prospect : toRelease) {
                team->releasePlayer(prospect);
                addFreeAgent(prospect);
            }
        }
    }

    void WorldData::processJuneThirtiethMidnightWipe() {
        for (auto& [id, player] : globalPlayerRegistry) {
            if (player) player->processMidnightWipe();
        }
    }

} // namespace FootballManager
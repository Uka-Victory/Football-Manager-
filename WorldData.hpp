#pragma once
#include "Player.hpp"
#include "Team.hpp"
#include <vector>
#include <string>
#include <memory>
#include <map>

namespace FootballManager {

    class WorldData {
    private:
        std::vector<PlayerPtr> freeAgentPool;
        std::vector<std::string> scoutWatchlist; // Stores Player IDs
        std::map<std::string, PlayerPtr> globalPlayerRegistry; // Master map of all active players

    public:
        WorldData();

        // Registry & Market
        void registerPlayer(PlayerPtr player);
        void addFreeAgent(PlayerPtr player);
        void removeFreeAgent(const std::string& playerId);
        PlayerPtr findPlayerGlobally(const std::string& playerId) const;
        std::vector<PlayerPtr> getFreeAgents() const { return freeAgentPool; }

        // Watchlist
        void addToWatchlist(const std::string& playerId);
        void removeFromWatchlist(const std::string& playerId);
        std::vector<std::string> getWatchlist() const { return scoutWatchlist; }

        // Temporal Engine Triggers
        void processAprilFirstGraduation(std::vector<std::shared_ptr<Team>>& allWorldTeams, int currentYear);
        void processJuneThirtiethMidnightWipe();
    };

} // namespace FootballManager
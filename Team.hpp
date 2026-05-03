#pragma once
#include "Player.hpp"
#include <vector>
#include <memory>
#include <map>
#include <stdexcept>

namespace FootballManager {

    using PlayerPtr = std::shared_ptr<Player>;

    class Team {
    private:
        int clubLevel; // 1-20
        int wageBudget;
        int currentWageSpend;

        std::vector<PlayerPtr> seniorSquad; // Cap: 25-35, Floor: 18
        std::vector<PlayerPtr> youthTeam;
        std::vector<PlayerPtr> academy;

        // Manual Depth Chart: Maps a Position to an ordered list of players
        std::map<Position, std::vector<PlayerPtr>> depthChart;

    public:
        Team(int level, int budget);

        // Core Roster Management
        void addPlayerToSenior(PlayerPtr player);
        void addPlayerToYouth(PlayerPtr player);
        void addPlayerToAcademy(PlayerPtr player);
        
        void promoteToSenior(PlayerPtr player);
        void demoteToYouth(PlayerPtr player);

        // Auto Squad Logic & Integrity
        void validateRoster(); // Enforces the 18-player hard floor
        std::vector<PlayerPtr> getBestXI();

        // Manual Hierarchy Management
        void setManualRank(Position pos, PlayerPtr player, int newRank);
        void resetPositionHierarchy(Position pos);

        // April 1st Logic
        std::vector<PlayerPtr> getGraduatingAcademyPlayers(int currentYear);
        void offerProfessionalContract(PlayerPtr player, int wage, int duration, bool toSenior);
        void releasePlayer(PlayerPtr player); // Hooks into WorldData Free Agent Pool externally
        
        // Finances
        int getAvailableWageBudget() const { return wageBudget - currentWageSpend; }
        int getClubLevel() const { return clubLevel; }
    };

} // namespace FootballManager
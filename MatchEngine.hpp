#pragma once
#include "Team.hpp"
#include "DataHub.hpp"
#include <vector>
#include <memory>
#include <string>
#include <random>

namespace FootballManager {

    struct MatchResult {
        int homeGoals = 0;
        int awayGoals = 0;
        double homeXG = 0.0;
        double awayXG = 0.0;
        std::vector<std::string> goalscorers;
    };

    class MatchEngine {
    private:
        DataHub& dataHub;
        std::mt19937 rng;

        // Tactical 12x8 Grid Boundaries
        const int GRID_X_MAX = 12;
        const int GRID_Y_MAX = 8;

        // Internal Simulation Methods
        PlayerPtr findWeakLink(const std::vector<PlayerPtr>& oppositionXI);
        double calculateDuelWinProbability(PlayerPtr attacker, PlayerPtr defender, int x, int y);
        double calculateShotXG(int x, int y, PlayerPtr shooter);
        double calculatePressureMultiplier(PlayerPtr player, bool isBigGame);

    public:
        MatchEngine(DataHub& hub);

        // Core Simulation
        MatchResult simulateMatch(Team& homeTeam, Team& awayTeam, bool isBigGame = false);
        
        // Automated Pre-Match Briefing
        std::string generateOppositionBriefing(Team& opposition);
    };

} // namespace FootballManager
// MatchEngine.hpp
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Team.hpp"
#include "Player.hpp"
#include "MatchStats.hpp"

// ========== MATCH ENGINE ==========
class MatchEngine {
public:
    static MatchResult simulateMatch(const TeamPtr& home, const TeamPtr& away);
    static void applyResultToTeams(const TeamPtr& home, const TeamPtr& away,
                                   MatchResult& res, const std::string& date);

private:
    static double computeAttackStrength(const std::vector<PlayerPtr>& xi);
    static double computeDefenceStrength(const std::vector<PlayerPtr>& xi);
    static double computeMidfieldControl(const std::vector<PlayerPtr>& xi);
    static double zoneThreat(int col, int row);
    static double shotXg(int col, int row, double pressure);
    static void simulatePossession(const std::vector<PlayerPtr>& attackXI,
                                   const std::vector<PlayerPtr>& defendXI,
                                   MatchResult& res,
                                   bool isHome,
                                   double homeControl, double awayControl);
    static double calculateMatchRating(const PlayerMatchStats& stats,
                                       const std::string& position);
};

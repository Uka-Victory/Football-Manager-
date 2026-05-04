// ProgressionEngine.hpp
#pragma once
#include <vector>
#include <memory>
#include "Player.hpp"
#include "Team.hpp"

class ProgressionEngine {
public:
    // Process all players in the world for one monthly tick.
    // activePlayers: all players currently in the game.
    // teams: all teams (needed for facilities and mentorship).
    // currentMonth, currentYear: for age checks.
    static void processMonthlyTick(
        std::vector<PlayerPtr>& activePlayers,
        const std::vector<TeamPtr>& teams,
        int currentMonth, int currentYear);

private:
    // Growth & decline phases
    static bool isInGrowthPhase(int age);
    static bool isInPeakPhase(int age);
    static bool isInDeclinePhase(int age);

    // Core progression score calculation
    static double calculateProgressionScore(
        const PlayerPtr& player,
        const TeamPtr& club,
        int matchesPlayedThisMonth,
        double avgRatingThisMonth,
        bool hasMentorBonus);

    // Determine which attribute to change (+1 or -1)
    static int selectAttributeForGrowth(const PlayerPtr& player,
                                        const std::map<std::string, double>& matchWeights);
    static int selectAttributeForDecline(const PlayerPtr& player);

    // Apply changes
    static void applyAttributeBump(PlayerPtr& player, int attrIndex, int delta);

    // Hidden potential ceiling adjustments
    static void checkInjuryCeilingReduction(PlayerPtr& player, bool severeInjuryThisMonth);
    static void checkLateBloomerCeilingExpansion(PlayerPtr& player, double seasonAvgRating, int careerApps);

    // Mentorship
    static bool hasMentorInTeam(const TeamPtr& team);
};
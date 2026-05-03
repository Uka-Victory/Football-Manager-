#pragma once
#include "Team.hpp"
#include <memory>

namespace FootballManager {

    struct MatchResult {
        int homeGoals;
        int awayGoals;
    };

    class MatchEngine {
    public:
        static MatchResult simulateMatch(std::shared_ptr<Team> homeTeam, std::shared_ptr<Team> awayTeam, bool isBigGame = false);
    };

} // namespace FootballManager
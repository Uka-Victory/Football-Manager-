#ifndef MATCHENGINE_HPP
#define MATCHENGINE_HPP

#include <vector>
#include <string>
#include "Team.hpp"

struct MatchResult {
    Team* homeTeam;
    Team* awayTeam;
    int homeGoals;
    int awayGoals;
    std::vector<std::string> homeScorers;
    std::vector<std::string> awayScorers;
};

class MatchEngine {
public:
    static MatchResult simulateMatch(Team& home, Team& away);
};

#endif
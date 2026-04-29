#ifndef MATCHENGINE_HPP
#define MATCHENGINE_HPP

#include "Team.hpp"
#include "League.hpp"

class MatchEngine {
public:
    MatchEngine() = default;

    // Takes the Smart Pointers of the two playing teams and the league they belong to.
    // It will calculate the score, apply fatigue to the players, update their match stats, 
    // and then automatically log the result in the league standings.
    void simulateMatch(TeamPtr homeTeam, TeamPtr awayTeam, LeaguePtr league);
};

#endif
#include "MatchEngine.hpp"
#include "Utils.hpp"

MatchResult MatchEngine::simulateMatch(Team& home, Team& away) {
    auto getStrength = [](Team& t) -> int {
        if (t.players.empty()) return 50;
        int sum = 0;
        for (const auto& p : t.players) sum += p.overall;
        return sum / static_cast<int>(t.players.size());
    };
    int homeStr = getStrength(home) + 5;
    int awayStr = getStrength(away);
    
    int homeGoals = (homeStr / 20) + randInt(0, 2);
    int awayGoals = (awayStr / 20) + randInt(0, 2);
    if (homeGoals < 0) homeGoals = 0;
    if (awayGoals < 0) awayGoals = 0;
    if (homeGoals > 5) homeGoals = 5;
    if (awayGoals > 5) awayGoals = 5;
    
    home.updateRecord(homeGoals, awayGoals);
    away.updateRecord(awayGoals, homeGoals);
    
    std::vector<std::string> homeScorers, awayScorers;
    for (int i = 0; i < homeGoals; i++) {
        if (!home.players.empty()) {
            int idx = randInt(0, static_cast<int>(home.players.size()) - 1);
            homeScorers.push_back(home.players[idx].name);
            home.players[idx].goals++;
        }
    }
    for (int i = 0; i < awayGoals; i++) {
        if (!away.players.empty()) {
            int idx = randInt(0, static_cast<int>(away.players.size()) - 1);
            awayScorers.push_back(away.players[idx].name);
            away.players[idx].goals++;
        }
    }
    return {&home, &away, homeGoals, awayGoals, homeScorers, awayScorers};
}
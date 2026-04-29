#include "MatchEngine.hpp"
#include "Utils.hpp"

void MatchEngine::simulateMatch(TeamPtr homeTeam, TeamPtr awayTeam, LeaguePtr league) {
    if (!homeTeam || !awayTeam || !league) return;

    // 1. Get average squad overalls
    int homeOvr = homeTeam->getTeamOverall();
    int awayOvr = awayTeam->getTeamOverall();

    // Small Home Advantage
    homeOvr += 3;

    // 2. Calculate scoring chances based on the difference in quality
    int diff = homeOvr - awayOvr;
    
    int homeChances = Utils::randInt(1, 5) + (diff > 0 ? diff / 4 : 0);
    int awayChances = Utils::randInt(1, 5) + (diff < 0 ? (-diff) / 4 : 0);

    if (homeChances < 0) homeChances = 0;
    if (awayChances < 0) awayChances = 0;

    // 3. Convert chances to goals (roughly 30% conversion rate)
    int homeGoals = 0;
    int awayGoals = 0;
    for (int i = 0; i < homeChances; i++) {
        if (Utils::randInt(0, 100) < 30) homeGoals++;
    }
    for (int i = 0; i < awayChances; i++) {
        if (Utils::randInt(0, 100) < 30) awayGoals++;
    }

    // 4. Record the result securely in the League
    league->recordMatch(homeTeam->getName(), awayTeam->getName(), homeGoals, awayGoals);

    // 5. Update Player Match Stats & Fatigue
    // (In V2, we will only apply this to the chosen Starting XI, but for now we apply random minutes to the whole squad)
    
    for (auto& p : homeTeam->getPlayers()) {
        int minsPlayed = Utils::randInt(0, 90);
        if (minsPlayed > 0) {
            p->applyFatigue(minsPlayed);
            // Basic match rating logic: base 6.0, boost if they won
            float rating = 6.0f + (Utils::randInt(0, 20) / 10.0f) + (homeGoals > awayGoals ? 1.0f : 0.0f);
            if (rating > 10.0f) rating = 10.0f;
            
            p->updateMatchStats(0, 0, (awayGoals == 0), (Utils::randInt(0, 100) > 95 ? 1 : 0), rating);
        }
    }

    for (auto& p : awayTeam->getPlayers()) {
        int minsPlayed = Utils::randInt(0, 90);
        if (minsPlayed > 0) {
            p->applyFatigue(minsPlayed);
            float rating = 6.0f + (Utils::randInt(0, 20) / 10.0f) + (awayGoals > homeGoals ? 1.0f : 0.0f);
            if (rating > 10.0f) rating = 10.0f;
            
            p->updateMatchStats(0, 0, (homeGoals == 0), (Utils::randInt(0, 100) > 95 ? 1 : 0), rating);
        }
    }
}
#include "MatchEngine.hpp"
#include <iostream>
#include <cmath>

namespace FootballManager {

    MatchEngine::MatchEngine(DataHub& hub) : dataHub(hub) {
        std::random_device rd;
        rng = std::mt19937(rd());
    }

    PlayerPtr MatchEngine::findWeakLink(const std::vector<PlayerPtr>& oppositionXI) {
        PlayerPtr weakLink = nullptr;
        double lowestRating = 10.0; // Max rating is 10.0

        for (const auto& player : oppositionXI) {
            if (player->getStats().appearances > 0 && player->getStats().averageRating < lowestRating) {
                lowestRating = player->getStats().averageRating;
                weakLink = player;
            }
        }
        return weakLink; // Can be null if it's the first game of the season
    }

    double MatchEngine::calculatePressureMultiplier(PlayerPtr player, bool isBigGame) {
        if (!isBigGame) return 1.0;
        // World Reputation (1-20) acts as a buffer against high-pressure drops
        int rep = player->getAttribute("composure"); // Proxy for hidden reputation access in engine context
        return 0.8 + (rep / 100.0); // Elite players suffer less penalty in big games
    }

    double MatchEngine::calculateDuelWinProbability(PlayerPtr attacker, PlayerPtr defender, int x, int y) {
        double attackScore = (attacker->getAttribute("pace") * 0.4) + (attacker->getAttribute("decisions") * 0.6);
        double defenseScore = (defender->getAttribute("tackling") * 0.7) + (defender->getAttribute("positioning") * 0.3);
        
        double total = attackScore + defenseScore;
        if (total == 0) return 0.5;
        return attackScore / total;
    }

    double MatchEngine::calculateShotXG(int x, int y, PlayerPtr shooter) {
        // Closer to goal (x approaching 12, y approaching 4/5) equals higher xG
        double distanceToGoal = std::sqrt(std::pow(12 - x, 2) + std::pow(4.5 - y, 2));
        double baseProbability = std::max(0.01, 1.0 - (distanceToGoal * 0.15));
        
        // Finishing attribute slightly modifies the expected quality of the chance taken
        double finishingMod = shooter->getAttribute("finishing") / 20.0;
        return baseProbability * (0.5 + (finishingMod * 0.5));
    }

    MatchResult MatchEngine::simulateMatch(Team& homeTeam, Team& awayTeam, bool isBigGame) {
        MatchResult result;
        std::vector<PlayerPtr> homeXI = homeTeam.getBestXI();
        std::vector<PlayerPtr> awayXI = awayTeam.getBestXI();

        // 1. Pre-Match Briefing Automations
        PlayerPtr awayWeakLink = findWeakLink(awayXI);
        PlayerPtr homeWeakLink = findWeakLink(homeXI);

        // 2. 90-Minute Simulation Loop
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        std::uniform_int_distribution<int> gridXDist(1, 12);
        std::uniform_int_distribution<int> gridYDist(1, 8);

        for (int minute = 1; minute <= 90; ++minute) {
            // Determine possession (simplified for engine speed)
            bool homePossession = dist(rng) > 0.5;
            std::vector<PlayerPtr>& attackingXI = homePossession ? homeXI : awayXI;
            std::vector<PlayerPtr>& defendingXI = homePossession ? awayXI : homeXI;
            
            // Generate a random event on the grid
            int eventX = gridXDist(rng);
            int eventY = gridYDist(rng);

            PlayerPtr activeAttacker = attackingXI[minute % 11];
            PlayerPtr activeDefender = defendingXI[(minute + 5) % 11];

            // Fatigue calculation
            double fatigueDrain = 0.5; 
            activeAttacker->setFitness(std::max(0, static_cast<int>(activeAttacker->getFitness() - fatigueDrain)));

            // Execute Duel
            double winProb = calculateDuelWinProbability(activeAttacker, activeDefender, eventX, eventY);
            if (homePossession && awayWeakLink == activeDefender) winProb *= 1.15; // Target weak link
            
            if (dist(rng) < winProb) {
                // Duel won, progression logged
                int passEndX = std::min(12, eventX + 2);
                if (homePossession) dataHub.logPassProgression(eventX, passEndX);
                activeAttacker->getStats().keyPasses++;

                // Chance Creation
                if (passEndX >= 10) { // Final third
                    PlayerPtr striker = attackingXI[10]; // Assuming index 10 is ST
                    double xG = calculateShotXG(passEndX, eventY, striker);
                    striker->getStats().accumulatedXG += xG;
                    
                    if (homePossession) result.homeXG += xG;
                    else result.awayXG += xG;

                    // Execute Shot against xG & Finishing
                    double shotQuality = (striker->getAttribute("finishing") * calculatePressureMultiplier(striker, isBigGame)) / 20.0;
                    if (dist(rng) < (xG * shotQuality * 2.5)) { 
                        if (homePossession) result.homeGoals++;
                        else result.awayGoals++;
                        
                        striker->getStats().goals++;
                        result.goalscorers.push_back(striker->getName() + " (" + std::to_string(minute) + "')");
                    }
                }
            } else {
                // Duel lost, log failure for Data Hub Forensic Map
                if (homePossession) dataHub.logDuelFailure(eventX, eventY);
            }
        }

        // Post-Match: Update appearances and moving average ratings (simplified rating generation)
        for (auto& player : homeXI) {
            player->getStats().appearances++;
            player->getStats().updateRating(6.0 + (dist(rng) * 3.0)); // Gen rating 6.0 - 9.0
        }
        for (auto& player : awayXI) {
            player->getStats().appearances++;
            player->getStats().updateRating(5.5 + (dist(rng) * 3.0));
        }

        return result;
    }

    std::string MatchEngine::generateOppositionBriefing(Team& opposition) {
        std::vector<PlayerPtr> xi = opposition.getBestXI();
        PlayerPtr weakLink = findWeakLink(xi);
        
        std::string brief = "=== OPPOSITION BRIEFING ===\n";
        brief += "Opponent Level: " + std::to_string(opposition.getClubLevel()) + " / 20\n";
        if (weakLink) {
            brief += "Tactical Target (Weak Link): " + weakLink->getName() + 
                     " (Avg Rating: " + std::to_string(weakLink->getStats().averageRating) + ")\n";
        } else {
            brief += "No clear weak link identified in recent form.\n";
        }
        return brief;
    }

} // namespace FootballManager
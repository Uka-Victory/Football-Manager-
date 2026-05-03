#include "MatchEngine.hpp"
#include "Utils.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace FootballManager {

    MatchResult MatchEngine::simulateMatch(std::shared_ptr<Team> homeTeam,
                                            std::shared_ptr<Team> awayTeam,
                                            bool isBigGame) {
        if (!homeTeam || !awayTeam) return {0, 0};

        auto homeXI = homeTeam->getBestXI();
        auto awayXI = awayTeam->getBestXI();

        if (homeXI.size() < 11 || awayXI.size() < 11) {
            std::cout << "[ERROR] Teams do not have 11 players. Match forfeit.\n";
            return {0, 0};
        }

        int homeGoals = 0, awayGoals = 0;

        // Fatigue modifier from infirmary level (0.6 - 1.0 range)
        float homeFatigueDecay = homeTeam->getFacilities()->getFatigueDecayModifier();
        float awayFatigueDecay  = awayTeam->getFacilities()->getFatigueDecayModifier();

        // Fix: isBigGame raises the score gap required to create a shot — pressure makes chances harder
        int shotThreshold = isBigGame ? 15 : 10;

        for (int minute = 1; minute <= 90; ++minute) {
            bool homePossession = (Utils::randInt(1, 100) > 50);
            auto& attackingXI   = homePossession ? homeXI : awayXI;
            auto& defendingXI   = homePossession ? awayXI : homeXI;

            // Index 0 is always the GK — outfield players are indices 1–10
            int activeAttackerIdx = Utils::randInt(1, 10);
            int activeDefenderIdx = Utils::randInt(1, 10);

            auto attacker = attackingXI[activeAttackerIdx];
            auto defender = defendingXI[activeDefenderIdx];

            int attackScore = attacker->getAttribute("passing")
                            + attacker->getAttribute("vision")
                            + Utils::randInt(1, 20);

            int defendScore = defender->getAttribute("tackling")
                            + defender->getAttribute("positioning")
                            + Utils::randInt(1, 20);

            // Fix: base drain is 2, multiplied by modifier and rounded.
            // std::round() prevents the cast-to-int truncation-to-zero bug.
            float attackFatigueMod = homePossession ? homeFatigueDecay : awayFatigueDecay;
            float defendFatigueMod = homePossession ? awayFatigueDecay  : homeFatigueDecay;

            int attackDrain = std::max(1, static_cast<int>(std::round(2.0f * attackFatigueMod)));
            int defendDrain = std::max(1, static_cast<int>(std::round(2.0f * defendFatigueMod)));

            attacker->setFitness(std::max(0, attacker->getFitness() - attackDrain));
            defender->setFitness(std::max(0, defender->getFitness() - defendDrain));

            // Shot event — threshold raised in big games
            if (attackScore > defendScore + shotThreshold) {
                attacker->getStats().accumulatedXG += 0.15;

                // Shot vs goalkeeper's positioning
                int finishRoll = attacker->getAttribute("finishing") + Utils::randInt(1, 20);
                int gkRoll     = defendingXI[0]->getAttribute("positioning") + Utils::randInt(1, 20);

                if (finishRoll > gkRoll) {
                    if (homePossession) homeGoals++; else awayGoals++;
                    attacker->getStats().goals++;
                }
            }
        }

        // Fix: updateRating() called BEFORE appearances++ so the moving average is correct
        for (auto& p : homeXI) {
            double rating = 6.0 + (p->getStats().goals * 1.5) + Utils::randDouble(-1.0, 1.0);
            p->getStats().updateRating(std::clamp(rating, 1.0, 10.0));
            p->getStats().appearances++;
        }
        for (auto& p : awayXI) {
            double rating = 6.0 + (p->getStats().goals * 1.5) + Utils::randDouble(-1.0, 1.0);
            p->getStats().updateRating(std::clamp(rating, 1.0, 10.0));
            p->getStats().appearances++;
        }

        return {homeGoals, awayGoals};
    }

} // namespace FootballManager
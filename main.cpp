#include "Utils.hpp"
#include "NamePool.hpp"
#include "TeamGenerator.hpp"
#include "MatchEngine.hpp"
#include "TrainingEngine.hpp"
#include "League.hpp"
#include <iostream>
#include <memory>

using namespace FootballManager;

void displayMenu(bool seasonDone) {
    std::cout << "\n=== FOOTBALL MANAGER ===\n";
    if (seasonDone) {
        std::cout << "  ** SEASON COMPLETE **\n";
    }
    std::cout << "1. Simulate Next Fixture\n";
    std::cout << "2. View Your Squad\n";
    std::cout << "3. View League Table\n";
    std::cout << "0. Quit\n";
    std::cout << "Select Option: ";
}

int main() {
    // Fix: initRNG must be called first so all Utils::randInt calls are truly random
    Utils::initRNG();

    NamePool namePool;
    League premierLeague("Premier League");

    // Build and populate teams
    auto playerClub = std::make_shared<Team>("Arsenal", 85);
    auto aiClub1    = std::make_shared<Team>("Chelsea", 82);
    auto aiClub2    = std::make_shared<Team>("Liverpool", 88);
    auto aiClub3    = std::make_shared<Team>("Man City", 90);

    TeamGenerator::populateTeam(playerClub, 25, namePool);
    TeamGenerator::populateTeam(aiClub1,    25, namePool);
    TeamGenerator::populateTeam(aiClub2,    25, namePool);
    TeamGenerator::populateTeam(aiClub3,    25, namePool);

    premierLeague.addTeam(playerClub);
    premierLeague.addTeam(aiClub1);
    premierLeague.addTeam(aiClub2);
    premierLeague.addTeam(aiClub3);
    premierLeague.generateRoundRobinSchedule();

    int  choice;
    bool running = true;

    while (running) {
        bool seasonDone = premierLeague.isSeasonComplete();
        displayMenu(seasonDone);

        if (!(std::cin >> choice)) break;

        switch (choice) {
            case 1: {
                // Fix: Use getNextFixture() so each simulation advances the schedule
                auto [home, away] = premierLeague.getNextFixture();

                if (!home || !away) {
                    std::cout << "All fixtures have been played. Season is over!\n";
                    break;
                }

                // Fix: Call TrainingEngine before each match so fitness recovers between games
                TrainingEngine::processDailyTraining(home);
                TrainingEngine::processDailyTraining(away);

                std::cout << "\n[MATCHDAY] " << home->getName()
                          << " vs " << away->getName() << "\n";

                // isBigGame is now actually used inside simulateMatch
                bool isBigGame = (home->getClubLevel() >= 85 && away->getClubLevel() >= 85);
                MatchResult result = MatchEngine::simulateMatch(home, away, isBigGame);

                premierLeague.recordMatchResult(home, away, result.homeGoals, result.awayGoals);

                std::cout << home->getName() << " "  << result.homeGoals
                          << " - " << result.awayGoals << " " << away->getName() << "\n";
                if (isBigGame) std::cout << "[BIG GAME — Chance creation threshold raised]\n";
                break;
            }
            case 2: {
                std::cout << "\n[YOUR SQUAD — " << playerClub->getName() << "]\n";
                for (const auto& player : playerClub->getSeniorSquad()) {
                    std::cout << "- " << player->getName()
                              << " | Fit: " << player->getFitness() << "%"
                              << " | Rating: " << player->getStats().averageRating << "\n";
                }
                break;
            }
            case 3:
                premierLeague.printTable();
                break;
            case 0:
                running = false;
                break;
            default:
                std::cout << "Invalid option.\n";
        }
    }

    std::cout << "Thanks for playing.\n";
    return 0;
}
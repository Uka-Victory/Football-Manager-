#include <iostream>
#include <string>
#include <memory>
#include "WorldData.hpp"
#include "GameCalendar.hpp"
#include "TeamGenerator.hpp"
#include "NamePool.hpp"
#include "MatchEngine.hpp"
#include "TransferEngine.hpp"
#include "Utils.hpp"

using namespace std;

// --- PROTOTYPES ---
void runGameLoop(WorldData& world, GameCalendar& calendar, TeamPtr myTeam, LeaguePtr myLeague);
void createNewGame(WorldData& world, TeamGenerator& generator, GameCalendar& calendar, TeamPtr& myTeam, LeaguePtr& myLeague);

int main() {
    Utils::initRNG();

    // 1. Initialize Global Tools
    NamePool namePool("first_names.txt", "last_names.txt"); 
    TeamGenerator generator(namePool);
    WorldData world;
    GameCalendar calendar;

    cout << "======================================\n";
    cout << "     FOOTBALL MANAGER: REBORN       \n";
    cout << "======================================\n";
    cout << "1. New Game\n";
    cout << "2. Load Game\n";
    cout << "3. Exit\n";
    cout << "Choice: ";
    
    int choice;
    cin >> choice;

    TeamPtr myTeam = nullptr;
    LeaguePtr myLeague = nullptr;

    if (choice == 1) {
        createNewGame(world, generator, calendar, myTeam, myLeague);
        runGameLoop(world, calendar, myTeam, myLeague);
    } 
    else if (choice == 2) {
        if (world.loadCareer("save_game.json")) {
            cout << "\nGame Loaded Successfully!\n";
            // For now, we just grab the first team and league to act as the player's team
            myLeague = world.getActiveLeagues().front();
            myTeam = myLeague->getTeams().front();
            
            // In a full implementation, GameCalendar would be saved/loaded via WorldData or a Master Save file.
            // For this test, we just start the loop!
            runGameLoop(world, calendar, myTeam, myLeague);
        } else {
            cout << "\nFailed to load game. Exiting...\n";
        }
    }

    return 0;
}

void createNewGame(WorldData& world, TeamGenerator& generator, GameCalendar& calendar, TeamPtr& myTeam, LeaguePtr& myLeague) {
    cout << "\nGenerating New World...\n";
    
    // 1. Create a Test League
    myLeague = make_shared<League>("Premier Division", 1);
    world.addLeagueToWorld(myLeague);

    // 2. Create and Populate Teams
    string teamNames[] = {"London FC", "Manchester Utd", "Liverpool FC", "Arsenal Blues"};
    for (const string& name : teamNames) {
        auto newTeam = make_shared<Team>(name, 1);
        generator.populateTeam(newTeam); // Generates 23-32 players!
        world.addTeamToWorld(newTeam);   // Adds to the O(1) global registry
        myLeague->addTeam(newTeam);      // Adds to the League standings
    }

    myTeam = myLeague->getTeamByName("London FC"); // Set your managed team
    
    // 3. Set starting date
    calendar = GameCalendar(2024, 7, 1); // Start July 1st, 2024
    
    cout << "World Generation Complete!\n";
}

void runGameLoop(WorldData& world, GameCalendar& calendar, TeamPtr myTeam, LeaguePtr myLeague) {
    MatchEngine matchEngine;
    TransferEngine transferEngine;
    bool playing = true;

    while (playing) {
        cout << "\n=== " << calendar.getDateString() << " ===\n";
        cout << "Manager of: " << myTeam->getName() << "\n";
        cout << "Balance: £" << myTeam->getBalance() << "\n";
        cout << "1. Advance 1 Day\n";
        cout << "2. View Squad\n";
        cout << "3. View League Table\n";
        cout << "4. Save Game\n";
        cout << "5. Quit to Desktop\n";
        cout << "Choice: ";
        
        int choice;
        cin >> choice;

        if (choice == 1) {
            calendar.advanceDay();
            
            // Simulate a random match day just for testing the engines
            if (calendar.getDay() % 7 == 0) {
                cout << "\n[Match Day! Simulating fixtures...]\n";
                auto teams = myLeague->getTeams();
                matchEngine.simulateMatch(teams[0], teams[1], myLeague);
                matchEngine.simulateMatch(teams[2], teams[3], myLeague);
            }

            if (calendar.isSeasonEnd()) {
                cout << "\n*** SEASON OVER ***\n";
                myLeague->resetSeason();
                cout << "Leagues have been reset for the new year.\n";
            }
        } 
        else if (choice == 2) {
            cout << "\n--- " << myTeam->getName() << " Squad ---\n";
            for (const auto& player : myTeam->getPlayers()) {
                cout << player->shortInfo() << " - Fitness: " << player->fitness << "%\n";
            }
        }
        else if (choice == 3) {
            cout << "\n--- " << myLeague->getName() << " Table ---\n";
            cout << "Team\t\tPts\tGD\n";
            for (const auto& row : myLeague->getSortedStandings()) {
                cout << row.first->getName() << "\t" << row.second.points << "\t" << row.second.goalDifference() << "\n";
            }
        }
        else if (choice == 4) {
            if (world.saveCareer("save_game.json")) {
                cout << "\n>>> Game Saved Successfully! <<<\n";
            }
        }
        else if (choice == 5) {
            playing = false;
        }
    }
}
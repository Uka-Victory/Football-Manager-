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
#include "TrainingEngine.hpp"

using namespace std;

// --- PROTOTYPES ---
void runGameLoop(WorldData& world, GameCalendar& calendar, TeamPtr myTeam, LeaguePtr myLeague);
void createNewGame(WorldData& world, TeamGenerator& generator, GameCalendar& calendar, TeamPtr& myTeam, LeaguePtr& myLeague);

int main() {
    Utils::initRNG(); // Fire up the new Random Number Generator

    NamePool namePool("first_names.txt", "last_names.txt"); 
    TeamGenerator generator(namePool);
    WorldData world;
    GameCalendar calendar;

    TeamPtr myTeam = nullptr;
    LeaguePtr myLeague = nullptr;

    cout << "======================================\n";
    cout << "     FOOTBALL MANAGER: REBORN       \n";
    cout << "======================================\n";
    cout << "1. New Game\n";
    cout << "2. Load Game\n";
    cout << "3. Exit\n";
    cout << "Choice: ";
    
    int choice;
    cin >> choice;

    if (choice == 1) {
        createNewGame(world, generator, calendar, myTeam, myLeague);
        runGameLoop(world, calendar, myTeam, myLeague);
    } 
    else if (choice == 2) {
        // We now pass the references so WorldData populates them perfectly!
        if (world.loadCareer("save_game.json", calendar, myTeam, myLeague)) {
            cout << "\nGame Loaded Successfully!\n";
            runGameLoop(world, calendar, myTeam, myLeague);
        } else {
            cout << "\nFailed to load game. Save file may be corrupted or missing.\n";
        }
    }

    return 0;
}

void createNewGame(WorldData& world, TeamGenerator& generator, GameCalendar& calendar, TeamPtr& myTeam, LeaguePtr& myLeague) {
    cout << "\nGenerating New World...\n";
    
    myLeague = make_shared<League>("Premier Division", 1);
    world.addLeagueToWorld(myLeague);

    string teamNames[] = {"London FC", "Manchester Utd", "Liverpool FC", "Arsenal Blues"};
    for (const string& name : teamNames) {
        auto newTeam = make_shared<Team>(name, 1);
        generator.populateTeam(newTeam); 
        world.addTeamToWorld(newTeam);   
        myLeague->addTeam(newTeam);      
    }

    myTeam = myLeague->getTeamByName("Arsenal Blues"); // Let's manage Arsenal Blues for testing!
    
    calendar = GameCalendar(2024, 7, 1); 
    
    cout << "World Generation Complete!\n";
}

void runGameLoop(WorldData& world, GameCalendar& calendar, TeamPtr myTeam, LeaguePtr myLeague) {
    MatchEngine matchEngine;
    TransferEngine transferEngine;
    TrainingEngine trainingEngine; // NEW: Instantiate the Training Engine
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
            
            // NEW: Run daily training and fitness recovery for the entire world
            for (const auto& [teamName, teamPtr] : world.getGlobalTeamRegistry()) {
                trainingEngine.processDailyTraining(teamPtr);
            }
            
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
            if (world.saveCareer("save_game.json", calendar, myTeam, myLeague)) {
                cout << "\n>>> Game Saved Successfully! <<<\n";
            }
        }
        else if (choice == 5) {
            playing = false;
        }
    }
}
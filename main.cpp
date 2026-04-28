#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <sstream>
#include "Player.hpp"
#include "Team.hpp"
#include "League.hpp"
#include "MatchEngine.hpp"
#include "TransferEngine.hpp"
#include "GameCalendar.hpp"
#include "Finances.hpp"
#include "Utils.hpp"
#include "WorldData.hpp"
#include "NamePool.hpp"
#include "TeamGenerator.hpp"

// YOUR ORIGINAL WAGE LOGIC
void generateInitialWageContracts(Team& team) {
    for (auto& p : team.players) {
        int base = 5000;
        if (p.overall >= 80) base = 150000;
        else if (p.overall >= 75) base = 90000;
        else if (p.overall >= 70) base = 50000;
        else if (p.overall >= 65) base = 25000;
        else if (p.overall >= 60) base = 12000;
        else if (p.overall >= 55) base = 6000;
        else if (p.overall >= 50) base = 3000;
        
        if (p.age <= 19) base = static_cast<int>(base * 0.5);
        else if (p.age >= 32) base = static_cast<int>(base * 0.8);
        
        int wage = static_cast<int>(base * randDouble(0.9, 1.1));
        team.finances.contracts[p.uniqueId] = {p.uniqueId, p.name, wage, "2028-06-30", false};
    }
}

void processWeeklyFinances(Team* team) {
    int totalWages = team->finances.getTotalWeeklyWages();
    team->finances.balance -= totalWages;
    team->finances.wageExpenses += totalWages;
}

int main() {
    // 1. DATA LOADING
    WorldData worldData;
    if (!worldData.loadFromJson("world_data.json")) return 1;
    NamePool namePool;
    if (!namePool.loadFromJson("countries_data.json")) return 1;
    TeamGenerator generator(namePool);
    TransferEngine transferEngine;
    GameCalendar calendar;
    std::vector<League> allLeagues;

    // 2. WORLD GENERATION
    std::cout << "--- Initializing Football World ---" << std::endl;
    auto countries = worldData.getAllCountries();
    for (auto const& [countryName, countryData] : countries) {
        for (auto const& lInfo : countryData.leagues) {
            League league(lInfo.name);
            for (auto const& tInfo : lInfo.teams) {
                Team temp = generator.generateTeam(tInfo.name, countryName, tInfo.level);
                Team* newTeam = new Team(temp.name, temp.country, temp.level, temp.finances.balance);
                newTeam->players = temp.players;
                newTeam->youthPlayers = temp.youthPlayers;
                generateInitialWageContracts(*newTeam);
                league.addTeam(newTeam);
            }
            league.generateSchedule("2025-08-01");
            allLeagues.push_back(std::move(league));
        }
    }
    std::cout << "Total Leagues Generated: " << allLeagues.size() << std::endl;

    // 3. TEAM SELECTION
    std::cout << "\n=== FOOTBALL MANAGER C++ ===\n";
    std::cout << "Select a League:\n";
    for (size_t i = 0; i < allLeagues.size(); ++i) 
        std::cout << i + 1 << ". " << allLeagues[i].name << "\n";
    
    int lIdx; std::cin >> lIdx;
    League* managedLeague = &allLeagues[lIdx - 1];

    std::cout << "\nSelect your Club in " << managedLeague->name << ":\n";
    for (size_t i = 0; i < managedLeague->teams.size(); ++i)
        std::cout << i + 1 << ". " << managedLeague->teams[i]->name << "\n";
    
    int tIdx; std::cin >> tIdx;
    Team* managedTeam = managedLeague->teams[tIdx - 1];

    // 4. MAIN GAME LOOP
    bool running = true;
    while (running) {
        std::string today = calendar.getDateString();
        std::cout << "\n========================================";
        std::cout << "\n DATE: " << today << " | " << managedTeam->name;
        std::cout << "\n========================================\n";

        // Check if managed team plays today
        bool matchToday = false;
        if (managedLeague->fixtures.count(today)) {
            for (auto& m : managedLeague->fixtures[today]) {
                if (m.homeTeam == managedTeam || m.awayTeam == managedTeam) matchToday = true;
            }
        }

        std::cout << "1. " << (matchToday ? "** PLAY MATCH **" : "Advance One Day") << "\n";
        std::cout << "2. View League Table\n";
        std::cout << "3. View Team Squad\n";
        std::cout << "4. Transfer Market\n";
        std::cout << "5. My Club Finances\n";
        std::cout << "6. Exit Game\n";
        std::cout << "Choice: ";

        int choice;
        if (!(std::cin >> choice)) break;

        switch (choice) {
            case 1: {
                // Simulate all matches for the whole world scheduled for today
                for (auto& league : allLeagues) {
                    if (league.fixtures.count(today)) {
                        for (auto& m : league.fixtures[today]) {
                            MatchResult res = MatchEngine::simulateMatch(*m.homeTeam, *m.awayTeam);
                            if (m.homeTeam == managedTeam || m.awayTeam == managedTeam) {
                                std::cout << "\n[FINAL RESULT] " << m.homeTeam->name << " " << res.homeGoals 
                                          << " - " << res.awayGoals << " " << m.awayTeam->name << "\n";
                            }
                        }
                    }
                }
                calendar.advanceOneDay();
                if (calendar.isSunday()) {
                    for (auto& l : allLeagues) {
                        for (auto* t : l.teams) processWeeklyFinances(t);
                    }
                }
                break;
            }
            case 2: managedLeague->printTable(); break;
            case 3:
                std::cout << "\n--- Squad List ---\n";
                for (auto& p : managedTeam->players) std::cout << p.shortInfo() << "\n";
                break;
            case 4:
                std::cout << "\nTotal players on market: " << transferEngine.getAvailableListings().size() << "\n";
                break;
            case 5:
                std::cout << "\nBalance: $" << managedTeam->finances.balance;
                std::cout << "\nWeekly Wages: $" << managedTeam->finances.getTotalWeeklyWages() << "\n";
                break;
            case 6: running = false; break;
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }

    // Cleanup
    for (auto& l : allLeagues) {
        for (auto* t : l.teams) delete t;
    }
    return 0;
}
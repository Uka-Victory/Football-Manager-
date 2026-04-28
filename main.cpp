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
        int wage = static_cast<int>(base * randDouble(0.85, 1.15));
        team.finances.contracts[p.uniqueId] = {p.uniqueId, p.name, wage, "2028-06-30", false};
    }
    for (auto& p : team.youthPlayers) {
        int base = 1500;
        if (p.overall >= 65) base = 6000;
        if (p.overall >= 70) base = 12000;
        int wage = static_cast<int>(base * randDouble(0.8, 1.1));
        team.finances.contracts[p.uniqueId] = {p.uniqueId, p.name, wage, "2028-06-30", true};
    }
}

void processWeeklyFinances(Team& team) {
    int totalWages = team.finances.getTotalWeeklyWages();
    team.finances.balance -= totalWages;
    team.finances.wageExpenses += totalWages;
    int sponsorship = 50000;
    if (team.finances.balance > 100000000) sponsorship = 500000;
    team.finances.balance += sponsorship;
    team.finances.sponsorshipIncome += sponsorship;
}

void processMatchdayFinances(Team& team, bool isHome) {
    if (!isHome) return;
    int income = 200000;
    if (team.finances.balance > 50000000) income = 500000;
    team.finances.balance += income;
    team.finances.matchdayIncome += income;
}

int main() {
    std::cout << "=== FOOTBALL MANAGER C++ ===\n" << std::endl;
    
    // Load name pool
    NamePool namePool;
    if (!namePool.loadFromJson("countries_data.json")) {
        std::cerr << "Failed to load names. Exiting.\n";
        return 1;
    }
    
    // Load world data (leagues and teams with levels)
    WorldData worldData;
    if (!worldData.loadFromJson("world_data.json")) {
        std::cerr << "Failed to load world data. Exiting.\n";
        return 1;
    }
    
    // Generate all teams using TeamGenerator
    TeamGenerator generator(namePool);
    std::vector<Team> teams;
    
    for (const auto& [countryName, countryData] : worldData.getAllCountries()) {
        for (const auto& league : countryData.leagues) {
            for (const auto& teamInfo : league.teams) {
                Team team = generator.generateTeam(teamInfo.name, countryName, teamInfo.level);
                generateInitialWageContracts(team);
                teams.push_back(team);
                std::cout << "Generated: " << team.name << " (Level " << team.level << ")\n";
            }
        }
    }
    
    std::cout << "\nGenerated " << teams.size() << " teams.\n";
    
    // Create a sample league (Premier League) for the UI – you can expand this
    League premierLeague("Premier League");
    for (auto& team : teams) {
        if (team.country == "England" && team.level >= 15) {
            premierLeague.addTeam(&team);
        }
    }
    
    GameCalendar calendar;
    TransferEngine transferEngine;
    
    bool running = true;
    int matchdayCounter = 1;
    
    // Simple main loop – you can select a managed team later
    Team* managedTeam = nullptr;
    if (!teams.empty()) {
        managedTeam = &teams[0]; // Just pick first team for demo
        std::cout << "\nManaged club automatically set to: " << managedTeam->name << "\n";
    }
    
    while (running) {
        std::cout << "\n--- Date: " << calendar.getDateString() << " ---\n";
        std::cout << "Transfer Window: " << (calendar.isTransferWindowOpen() ? "OPEN" : "CLOSED") << "\n";
        std::cout << "1. View League Table\n";
        std::cout << "2. View Team Squads\n";
        std::cout << "3. Simulate Matchday\n";
        std::cout << "4. Transfer Market\n";
        std::cout << "5. My Club Finances\n";
        std::cout << "6. Advance One Day\n";
        std::cout << "7. Exit\n";
        std::cout << "Choice: ";
        
        int choice;
        std::cin >> choice;
        std::cin.ignore();
        
        switch (choice) {
            case 1:
                premierLeague.printTable();
                break;
                
            case 2:
                for (auto& team : teams) {
                    std::cout << "\n=== " << team.name << " ===\n";
                    for (const auto& p : team.players) {
                        std::cout << "  " << p.shortInfo() << "\n";
                    }
                }
                break;
                
            case 3: {
                std::cout << "\n--- Matchday " << matchdayCounter++ << " Results ---\n";
                // Simulate a few random matches from the premier league
                auto tableTeams = premierLeague.getSortedTable();
                if (tableTeams.size() >= 2) {
                    for (size_t i = 0; i < tableTeams.size(); i += 2) {
                        if (i + 1 < tableTeams.size()) {
                            auto result = MatchEngine::simulateMatch(*tableTeams[i], *tableTeams[i+1]);
                            std::cout << result.homeTeam->name << " " << result.homeGoals << " - " << result.awayGoals << " " << result.awayTeam->name << "\n";
                            processMatchdayFinances(*result.homeTeam, true);
                            processMatchdayFinances(*result.awayTeam, false);
                        }
                    }
                }
                break;
            }
            
            case 4: {
                std::cout << "\n=== TRANSFER MARKET ===\n";
                auto listings = transferEngine.getAvailableListings();
                if (listings.empty()) {
                    std::cout << "No players listed.\n";
                } else {
                    for (size_t i = 0; i < listings.size(); i++) {
                        const auto& l = listings[i];
                        std::cout << i+1 << ". " << l.playerName << " (" << l.position << ") - " << l.currentClub
                                  << " | Asking: $" << l.askingPrice << " | Wage: $" << l.weeklyWage << "/week\n";
                    }
                    if (managedTeam) {
                        std::cout << "Enter number to bid (0 to cancel): ";
                        int bidChoice;
                        std::cin >> bidChoice;
                        if (bidChoice > 0 && bidChoice <= (int)listings.size()) {
                            const auto& l = listings[bidChoice-1];
                            std::cout << "Enter bid amount: ";
                            int amount;
                            std::cin >> amount;
                            std::cout << "Enter wage offer: ";
                            int wage;
                            std::cin >> wage;
                            transferEngine.submitBid(l.playerId, managedTeam->name, amount, wage, calendar.getDateString());
                            std::cout << "Bid submitted.\n";
                        }
                    }
                }
                if (managedTeam) {
                    std::cout << "\nList a player? (y/n): ";
                    char listChoice;
                    std::cin >> listChoice;
                    if (listChoice == 'y') {
                        std::cout << "Enter player name: ";
                        std::string pname;
                        std::cin.ignore();
                        std::getline(std::cin, pname);
                        Player* p = managedTeam->findPlayer(pname);
                        if (p) {
                            std::cout << "Asking price: ";
                            int price;
                            std::cin >> price;
                            int wage = 5000;
                            auto it = managedTeam->finances.contracts.find(p->uniqueId);
                            if (it != managedTeam->finances.contracts.end()) wage = it->second.weeklyWage;
                            transferEngine.listPlayerForTransfer(p, managedTeam->name, price, calendar.getDateString(), wage);
                            std::cout << p->name << " listed for transfer.\n";
                        } else {
                            std::cout << "Player not found.\n";
                        }
                    }
                }
                break;
            }
            
            case 5: {
                if (managedTeam) {
                    auto& fin = managedTeam->finances;
                    std::cout << "\n=== " << managedTeam->name << " Finances ===\n";
                    std::cout << "Balance: $" << fin.balance << "\n";
                    std::cout << "Transfer Budget: $" << fin.transferBudget << "\n";
                    std::cout << "Wage Budget: $" << fin.wageBudget << "/week\n";
                    std::cout << "Current Wages: $" << fin.getTotalWeeklyWages() << "/week\n";
                } else {
                    std::cout << "No managed club selected.\n";
                }
                break;
            }
            
            case 6: {
                calendar.advanceOneDay();
                if (calendar.isSunday()) {
                    for (auto& team : teams) processWeeklyFinances(team);
                }
                if (calendar.isTransferWindowOpen()) {
                    transferEngine.processAIActivity(teams, calendar.getDateString());
                }
                std::cout << "Advanced to " << calendar.getDateString() << "\n";
                break;
            }
            
            case 7:
                running = false;
                break;
                
            default:
                std::cout << "Invalid choice.\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }
    
    std::cout << "Thanks for playing!\n";
    return 0;
}
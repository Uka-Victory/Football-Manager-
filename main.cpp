#include "Team.hpp"
#include "DataHub.hpp"
#include "WorldData.hpp"
#include "MatchEngine.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace FootballManager;

// Helper to format dates
std::string getDateString(int month, int day, int year) {
    return std::to_string(year) + "-" + (month < 10 ? "0" : "") + std::to_string(month) + "-" + (day < 10 ? "0" : "") + std::to_string(day);
}

int main() {
    std::cout << "Initializing FootballManager Engine...\n";

    // 1. Boot up Global Registries
    WorldData worldData;
    DataHub dataHub;
    MatchEngine matchEngine(dataHub);

    // 2. Initialize Clubs (1-20 Scale)
    auto playerClub = std::make_shared<Team>(15, 500000); // Level 15 Club, 500k wage budget
    auto aiClub = std::make_shared<Team>(14, 400000);
    std::vector<std::shared_ptr<Team>> allTeams = {playerClub, aiClub};

    // Note: In a full deployment, TeamGenerator would populate these rosters here.
    // For this engine core, we assume rosters are populated to valid >18 numbers.
    // playerClub->addPlayerToSenior(std::make_shared<Player>(...));

    // 3. Temporal Engine Variables
    int currentYear = 2026;
    int currentMonth = 7; // Starts in July
    int currentDay = 1;

    bool gameRunning = true;

    // 4. Main Game Loop
    while (gameRunning) {
        std::cout << "\n========================================\n";
        std::cout << "DATE: " << getDateString(currentMonth, currentDay, currentYear) << "\n";
        std::cout << "WAGE BUDGET: " << playerClub->getAvailableWageBudget() << " remaining\n";
        std::cout << "========================================\n";
        std::cout << "[1] Advance 1 Day\n";
        std::cout << "[2] Simulate Next Match\n";
        std::cout << "[3] View Senior Squad & Hierarchy\n";
        std::cout << "[8] View Academy\n";
        std::cout << "[10] Data Hub\n";
        std::cout << "[0] Quit\n";
        std::cout << "Select Option: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear(); std::cin.ignore(10000, '\n'); continue;
        }

        switch (choice) {
            case 1: {
                // Temporal Progression
                currentDay++;
                if (currentDay > 30) { // Simplified 30-day months for calendar speed
                    currentDay = 1;
                    currentMonth++;
                    if (currentMonth > 12) {
                        currentMonth = 1;
                        currentYear++;
                    }
                }

                // Event Trigger: April 1st Graduation
                if (currentMonth == 4 && currentDay == 1) {
                    std::cout << "\n*** MANDATORY BOARD ACTION: ACADEMY GRADUATION ***\n";
                    worldData.processAprilFirstGraduation(allTeams, currentYear);
                    // UI interaction to sign/release prospects would pause here.
                    std::cout << "Graduation processing complete. Unsigned players moved to Free Agents.\n";
                }

                // Event Trigger: June 30th Midnight Wipe
                if (currentMonth == 6 && currentDay == 30) {
                    std::cout << "\n*** SEASON END: VOLATILE STAT BUFFER WIPE ***\n";
                    worldData.processJuneThirtiethMidnightWipe();
                    dataHub.resetSeasonalData();
                    std::cout << "All seasonal stats have been reset to zero.\n";
                }
                break;
            }
            case 2: {
                try {
                    std::cout << "\n" << matchEngine.generateOppositionBriefing(*aiClub);
                    std::cout << "Simulating Match...\n";
                    MatchResult res = matchEngine.simulateMatch(*playerClub, *aiClub);
                    std::cout << "RESULT: Home " << res.homeGoals << " - " << res.awayGoals << " Away\n";
                    std::cout << "xG:     Home " << res.homeXG << " - " << res.awayXG << " Away\n";
                } catch (const std::exception& e) {
                    std::cout << "MATCH CANCELLED: " << e.what() << "\n";
                }
                break;
            }
            case 3: {
                std::cout << "\n--- SENIOR SQUAD HIERARCHY ---\n";
                std::cout << "[M] Manage Hierarchy (Feature accessible via UI integration)\n";
                std::cout << "To swap LB1 and LB2, select player ID and input new Manual Rank.\n";
                // Roster printing logic goes here
                break;
            }
            case 10: {
                std::cout << "\n--- DATA HUB ---\n";
                std::cout << "Verticality Index: " << dataHub.getVerticalityIndex() << " grid units per pass\n";
                std::cout << "Clinical Delta: " << dataHub.calculateClinicalDelta(*playerClub) << "\n";
                
                std::cout << "\nForensic Grid Failures (Top 5):\n";
                auto softZones = dataHub.getSoftZones();
                for (const auto& zone : softZones) {
                    std::cout << "Coordinate (" << zone.first.first << ", " << zone.first.second << ") - Failures: " << zone.second << "\n";
                }

                std::cout << "\nHierarchy Conflict Report:\n";
                auto hReport = dataHub.generateHierarchyReport(*playerClub);
                for (const auto& rep : hReport) {
                    if (rep.isConflict) {
                        std::cout << "WARNING: " << rep.playerName << " (Star Player) is manually ranked dangerously low.\n";
                    }
                }
                break;
            }
            case 0:
                gameRunning = false;
                break;
            default:
                std::cout << "Invalid option.\n";
                break;
        }
    }

    std::cout << "Engine Shutdown.\n";
    return 0;
}
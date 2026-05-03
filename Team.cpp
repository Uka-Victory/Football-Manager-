#include "Team.hpp"
#include <algorithm>

namespace FootballManager {

    Team::Team(int level, int budget) : clubLevel(level), wageBudget(budget), currentWageSpend(0) {
        // Initialize empty depth charts for all positions
        depthChart[Position::GK] = {};
        depthChart[Position::CB] = {};
        depthChart[Position::LB] = {};
        depthChart[Position::RB] = {};
        depthChart[Position::DM] = {};
        depthChart[Position::CM] = {};
        depthChart[Position::AM] = {};
        depthChart[Position::RW] = {};
        depthChart[Position::LW] = {};
        depthChart[Position::ST] = {};
    }

    void Team::addPlayerToSenior(PlayerPtr player) {
        if (seniorSquad.size() >= 35) {
            throw std::runtime_error("Senior squad is at the 35 player maximum cap.");
        }
        seniorSquad.push_back(player);
        currentWageSpend += player->getWeeklyWage();
        depthChart[player->getPrimaryPosition()].push_back(player);
    }

    void Team::addPlayerToYouth(PlayerPtr player) {
        youthTeam.push_back(player);
        currentWageSpend += player->getWeeklyWage();
    }

    void Team::addPlayerToAcademy(PlayerPtr player) {
        academy.push_back(player);
    }

    void Team::promoteToSenior(PlayerPtr player) {
        auto it = std::find(youthTeam.begin(), youthTeam.end(), player);
        if (it != youthTeam.end()) {
            youthTeam.erase(it);
            addPlayerToSenior(player);
        }
    }

    void Team::demoteToYouth(PlayerPtr player) {
        auto it = std::find(seniorSquad.begin(), seniorSquad.end(), player);
        if (it != seniorSquad.end()) {
            // Remove from depth chart
            auto& posChart = depthChart[player->getPrimaryPosition()];
            posChart.erase(std::remove(posChart.begin(), posChart.end(), player), posChart.end());
            
            seniorSquad.erase(it);
            addPlayerToYouth(player);
        }
    }

    void Team::validateRoster() {
        // Enforce the 18-player Hard Floor
        while (seniorSquad.size() < 18) {
            if (youthTeam.empty()) {
                throw std::runtime_error("Critical Roster Failure: Less than 18 senior players and youth team is empty.");
            }
            // Emergency Promotion: Find the youth player with highest Selection Index
            auto bestYouthIt = std::max_element(youthTeam.begin(), youthTeam.end(),
                [](const PlayerPtr& a, const PlayerPtr& b) {
                    return a->getSelectionIndex() < b->getSelectionIndex();
                });
            
            PlayerPtr emergencyProspect = *bestYouthIt;
            emergencyProspect->setStatus(SquadStatus::EmergencyProspect);
            promoteToSenior(emergencyProspect);
        }
    }

    void Team::setManualRank(Position pos, PlayerPtr player, int newRank) {
        auto& posChart = depthChart[pos];
        auto it = std::find(posChart.begin(), posChart.end(), player);
        
        if (it != posChart.end()) {
            posChart.erase(it);
            // Insert at the new rank index (0-based)
            if (newRank <= 0) {
                posChart.insert(posChart.begin(), player);
            } else if (newRank >= posChart.size()) {
                posChart.push_back(player);
            } else {
                posChart.insert(posChart.begin() + newRank, player);
            }
            
            // Sync the manualRank integer in the Player objects to match their new index
            for (size_t i = 0; i < posChart.size(); ++i) {
                posChart[i]->setManualRank(static_cast<int>(i) + 1);
            }
        }
    }

    void Team::resetPositionHierarchy(Position pos) {
        auto& posChart = depthChart[pos];
        // Sort strictly by Selection Index (Attributes + Fitness + Status)
        std::sort(posChart.begin(), posChart.end(), [](const PlayerPtr& a, const PlayerPtr& b) {
            return a->getSelectionIndex() > b->getSelectionIndex();
        });
        
        // Reset manual flags
        for (auto& player : posChart) {
            player->setManualRank(99); 
        }
    }

    std::vector<PlayerPtr> Team::getBestXI() {
        validateRoster(); // Ensure 18 players exist
        std::vector<PlayerPtr> startingXI;
        std::vector<Position> tactic = {
            Position::GK, Position::RB, Position::CB, Position::CB, Position::LB,
            Position::DM, Position::CM, Position::CM, Position::RW, Position::LW, Position::ST
        };

        std::vector<PlayerPtr> availablePlayers = seniorSquad;

        for (Position pos : tactic) {
            PlayerPtr selected = nullptr;
            const auto& posChart = depthChart[pos];

            // 1. Check Manual Depth Chart first
            for (const auto& player : posChart) {
                auto it = std::find(availablePlayers.begin(), availablePlayers.end(), player);
                if (it != availablePlayers.end() && player->getFitness() >= 60) {
                    selected = player;
                    availablePlayers.erase(it);
                    break;
                }
            }

            // 2. Fallback to AI Selection if depth chart failed or fitness was too low
            if (!selected) {
                double bestIndex = -1.0;
                auto bestIt = availablePlayers.end();

                for (auto it = availablePlayers.begin(); it != availablePlayers.end(); ++it) {
                    if ((*it)->getPrimaryPosition() == pos && (*it)->getFitness() >= 60) {
                        double idx = (*it)->getSelectionIndex();
                        if (idx > bestIndex) {
                            bestIndex = idx;
                            bestIt = it;
                        }
                    }
                }
                if (bestIt != availablePlayers.end()) {
                    selected = *bestIt;
                    availablePlayers.erase(bestIt);
                }
            }

            if (selected) {
                startingXI.push_back(selected);
            }
        }
        return startingXI;
    }

    std::vector<PlayerPtr> Team::getGraduatingAcademyPlayers(int currentYear) {
        std::vector<PlayerPtr> graduates;
        for (const auto& player : academy) {
            // One Year Residency Rule
            if (currentYear - player->getAcademyJoinYear() >= 1) {
                graduates.push_back(player);
            }
        }
        return graduates;
    }

    void Team::offerProfessionalContract(PlayerPtr player, int wage, int duration, bool toSenior) {
        if (getAvailableWageBudget() < wage) {
            throw std::runtime_error("Insufficient wage budget for this contract.");
        }
        
        auto it = std::find(academy.begin(), academy.end(), player);
        if (it != academy.end()) {
            academy.erase(it);
            player->assignContract(wage, duration);
            if (toSenior) {
                addPlayerToSenior(player);
            } else {
                addPlayerToYouth(player);
            }
        }
    }

    void Team::releasePlayer(PlayerPtr player) {
        // Logic will remove player from vectors. 
        // WorldData will catch the released pointer and add to freeAgentPool.
        auto itAcademy = std::find(academy.begin(), academy.end(), player);
        if (itAcademy != academy.end()) academy.erase(itAcademy);

        auto itYouth = std::find(youthTeam.begin(), youthTeam.end(), player);
        if (itYouth != youthTeam.end()) youthTeam.erase(itYouth);

        auto itSenior = std::find(seniorSquad.begin(), seniorSquad.end(), player);
        if (itSenior != seniorSquad.end()) {
            auto& posChart = depthChart[player->getPrimaryPosition()];
            posChart.erase(std::remove(posChart.begin(), posChart.end(), player), posChart.end());
            seniorSquad.erase(itSenior);
        }
    }

} // namespace FootballManager
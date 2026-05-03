#include "Team.hpp"
#include <algorithm>

namespace FootballManager {

    Team::Team(std::string teamName, int level)
        : name(teamName), clubLevel(level) {
        finances   = std::make_shared<Finances>(50000000, 1000000);
        facilities = std::make_shared<Facilities>(10, 10, 30000, 10);
    }

    void Team::addPlayerToSenior(std::shared_ptr<Player> player) {
        if (player) {
            seniorSquad.push_back(player);
            finances->addWage(player->getWeeklyWage());
        }
    }

    void Team::releasePlayer(std::shared_ptr<Player> player) {
        auto it = std::find(seniorSquad.begin(), seniorSquad.end(), player);
        if (it != seniorSquad.end()) {
            finances->removeWage((*it)->getWeeklyWage());
            seniorSquad.erase(it);
        }
    }

    std::vector<std::shared_ptr<Player>> Team::getBestXI() {
        std::vector<std::shared_ptr<Player>> startingXI;
        std::vector<std::shared_ptr<Player>> available = seniorSquad;

        if (available.empty()) return startingXI;

        // Fix: Pick the best GK first and lock them to index 0
        auto gkIt = std::find_if(available.begin(), available.end(),
            [](const std::shared_ptr<Player>& p) {
                return p->getPrimaryPosition() == Position::GK;
            });

        if (gkIt != available.end()) {
            startingXI.push_back(*gkIt);
            available.erase(gkIt);
        } else {
            // No GK in squad — emergency: use lowest-rated player as keeper
            startingXI.push_back(available.front());
            available.erase(available.begin());
        }

        // Fill remaining 10 spots with the highest selection-index outfield players
        std::sort(available.begin(), available.end(),
            [](const std::shared_ptr<Player>& a, const std::shared_ptr<Player>& b) {
                return a->getSelectionIndex() > b->getSelectionIndex();
            });

        for (size_t i = 0; i < std::min((size_t)10, available.size()); ++i) {
            startingXI.push_back(available[i]);
        }

        return startingXI;
    }

} // namespace FootballManager
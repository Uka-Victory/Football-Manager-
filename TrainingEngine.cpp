#include "TrainingEngine.hpp"
#include <algorithm>

namespace FootballManager {

    void TrainingEngine::processDailyTraining(std::shared_ptr<Team> team) {
        if (!team) return;

        auto roster = team->getSeniorSquad();
        for (auto& player : roster) {
            // Restore fitness daily based on 1-100 scale, completely avoiding phantom 'injured' vars
            int currentFitness = player->getFitness();
            if (currentFitness < 100) {
                player->setFitness(std::min(100, currentFitness + 5));
            }
        }
    }

} // namespace FootballManager
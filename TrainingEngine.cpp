// TrainingEngine.cpp
#include "TrainingEngine.hpp"
#include "Utils.hpp"

void TrainingEngine::processDailyTraining(const TeamPtr& team) {
    for (auto& p : team->getSeniorSquad()) {
        if (p->isAvailable()) p->recoverFitness(5 + team->getFacilities().training * 2);
    }
    for (auto& p : team->getYouthSquad()) {
        if (p->isAvailable()) p->recoverFitness(5 + team->getFacilities().training * 2);
    }
}
#ifndef TRAININGENGINE_HPP
#define TRAININGENGINE_HPP

#include "Team.hpp"

class TrainingEngine {
public:
    TrainingEngine() = default;

    // To be called during the daily Game Loop. 
    // Recovers fitness, applies micro-growth to attributes, and rolls for training injuries.
    void processDailyTraining(TeamPtr team);
};

#endif
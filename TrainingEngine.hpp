#ifndef TRAININGENGINE_HPP
#define TRAININGENGINE_HPP

#include "Team.hpp"

class TrainingEngine {
public:
    // Processes fitness recovery, injury rolls, and CA/PA growth based on Facility levels
    static void processDailyTraining(TeamPtr team);
};

#endif
#pragma once
#include "Team.hpp"
#include <memory>

namespace FootballManager {

    class TrainingEngine {
    public:
        static void processDailyTraining(std::shared_ptr<Team> team);
    };

} // namespace FootballManager
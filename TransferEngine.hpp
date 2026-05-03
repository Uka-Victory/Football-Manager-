#pragma once
#include "Team.hpp"
#include <vector>
#include <memory>

namespace FootballManager {

    class TransferEngine {
    public:
        static void processAITransfers(std::vector<std::shared_ptr<Team>>& allTeams);
    };

} // namespace FootballManager
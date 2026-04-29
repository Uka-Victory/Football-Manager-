#ifndef TRANSFERENGINE_HPP
#define TRANSFERENGINE_HPP

#include <string>
#include <cstdint>
#include "Team.hpp"
#include "WorldData.hpp"

class TransferEngine {
public:
    TransferEngine() = default;

    // Executes a transfer between two clubs using their smart pointers
    // Uses int64_t to safely handle massive real-world transfer fees
    bool completeTransfer(TeamPtr buyingTeam, TeamPtr sellingTeam, const std::string& playerId, int64_t transferFee, int64_t newWage);

    // AI logic for computer-controlled teams to buy and sell players
    // Takes the entire WorldData by reference so the AI can scout the global player registry
    void processAITransfers(WorldData& world);
};

#endif
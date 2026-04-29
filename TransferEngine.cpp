#include "TransferEngine.hpp"
#include "Utils.hpp"

bool TransferEngine::completeTransfer(TeamPtr buyingTeam, TeamPtr sellingTeam, const std::string& playerId, int64_t transferFee, int64_t newWage) {
    if (!buyingTeam || !sellingTeam) return false; // Safety check

    // 1. O(1) Instant Lookup: Grab the player from the selling team
    PlayerPtr p = sellingTeam->getPlayerById(playerId);
    if (!p) return false;

    // 2. Financial Check
    if (buyingTeam->getTransferBudget() < transferFee) {
        return false; // Buying team cannot afford the transfer
    }

    // 3. Move the Money
    // Deduct from the buying team's transfer allowance and main bank balance
    buyingTeam->setBudgets(buyingTeam->getTransferBudget() - transferFee, buyingTeam->getWageBudget());
    buyingTeam->deductFunds(transferFee); 
    
    // Add the fee to the selling team's main bank balance
    sellingTeam->addFunds(transferFee);

    // 4. Update the Player's Contract
    p->wage = newWage;
    p->transferStatus = "Not Listed";

    // 5. The Memory Swap! 
    // We add the pointer to the new team BEFORE removing it from the old team 
    // to ensure the smart pointer's reference count never drops to 0 mid-transfer.
    buyingTeam->addPlayer(p);
    sellingTeam->removePlayer(playerId);

    return true;
}

void TransferEngine::processAITransfers(WorldData& world) {
    // PHASE 1 PLACEHOLDER
    // The architecture is now ready to support AI transfers.
    // In Phase 2, we will write logic here that allows AI teams to scout the 
    // global registry, identify weak spots in their squad, and submit bids.
}
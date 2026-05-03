#ifndef TRANSFERENGINE_HPP
#define TRANSFERENGINE_HPP

#include "Team.hpp"
#include "Player.hpp"
#include <vector>

struct TransferBid {
    TeamPtr buyer;
    TeamPtr seller;
    PlayerPtr player;
    int64_t amount = 0;
    bool isLoan = false;
    int loanMonths = 0;
    bool accepted = false;
};

class TransferEngine {
public:
    // Computes dynamic valuation based on CA, age, contract length, and home-grown status
    static int64_t calculateMarketValue(const PlayerPtr& p, const TeamPtr& buyingTeam = nullptr);
    
    // The Social Dilemma Loop: AI managers calculate risk and make bids
    static void processAITransfers(std::vector<TeamPtr>& allTeams);
    
    // Executes the financial and roster shifts if a bid is accepted
    static bool completeTransfer(TransferBid& bid);
};

#endif
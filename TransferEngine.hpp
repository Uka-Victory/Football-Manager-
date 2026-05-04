// TransferEngine.hpp
#pragma once
#include "Team.hpp"
#include "Player.hpp"
#include <vector>
#include <string>

struct TransferBid {
    TeamPtr buyer;
    TeamPtr seller;
    PlayerPtr player;
    int64_t amount;
    bool isLoan = false;
    int loanMonths = 0;
    int wageSplitPercent = 50;  // selling club pays this %
    bool accepted = false;
    bool rejected = false;
};

class TransferEngine {
public:
    static int64_t calculateMarketValue(const PlayerPtr& p, const TeamPtr& buyingTeam = nullptr);
    static int calculateWeeklyWage(const PlayerPtr& p);
    static void processAITransfers(std::vector<TeamPtr>& allTeams,
                                   std::vector<TransferBid>& activeBids,
                                   int currentYear, int currentMonth);
    static bool isTransferWindowOpen(int month);
    static void completeTransfer(TransferBid& bid);
    static void negotiateBid(TransferBid& bid, int64_t counterOffer);
private:
    static double getAgeFactor(int age);
    static double getContractFactor(int monthsRemaining);
    static double getFormFactor(const PlayerPtr& p);
    static double getHomeGrownFactor(const PlayerPtr& p, const TeamPtr& buyer);
    static double getPlaystyleMultiplier(Playstyle ps);
    static double getTraitMultiplier(const std::vector<Trait>& traits);
};
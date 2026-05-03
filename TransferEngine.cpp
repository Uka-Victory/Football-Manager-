#include "TransferEngine.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cmath>

int64_t TransferEngine::calculateMarketValue(const PlayerPtr& p, const TeamPtr& buyingTeam) {
    if (!p) return 0;

    // Base value derived from Current Ability (1-200) and Reputation (1-20)
    double baseValue = std::pow(p->currentAbility, 2.5) * 50.0 + (p->reputation * 100000.0);

    // Age Curve: Wonderkids are expensive, older players lose value rapidly
    double ageFactor = 1.0;
    if (p->age < 22) {
        double potentialGap = p->potentialAbility - p->currentAbility;
        ageFactor = 1.0 + (potentialGap * 0.02); // Multiplier for high potential ceiling
    } else if (p->age > 30) {
        ageFactor = std::max(0.2, 1.0 - ((p->age - 30) * 0.15));
    }

    // Contract Leverage: 1 year left = cheaper. 4+ years left = premium.
    double contractFactor = 1.0;
    if (p->contractLengthMonths <= 12) contractFactor = 0.6;
    else if (p->contractLengthMonths >= 48) contractFactor = 1.4;

    // Home-grown premium calculation
    double homeGrownFactor = 1.0;
    if (buyingTeam && p->homeGrownNation == buyingTeam->getCountry()) homeGrownFactor = 1.3;

    int64_t finalValue = static_cast<int64_t>(baseValue * ageFactor * contractFactor * homeGrownFactor);
    
    // Round to nearest £100k
    return (finalValue / 100000) * 100000;
}

void TransferEngine::processAITransfers(std::vector<TeamPtr>& allTeams) {
    for (auto& buyingTeam : allTeams) {
        if (buyingTeam->finances.transferEmbargo) continue; 
        if (buyingTeam->finances.transferBudget < 500000) continue; 

        // AI Manager Personas
        bool isDesperate = buyingTeam->finances.getBoardConfidence() < 40;
        bool isProjectBuilder = !isDesperate && buyingTeam->getLevel() < 15;
        
        // 5% chance per tick for an AI team to initiate a bid
        if (Utils::randDouble() < 0.05) { 
            TeamPtr sellingTeam = allTeams[Utils::randInt(0, allTeams.size() - 1)];
            if (buyingTeam == sellingTeam || sellingTeam->getSeniorSquad().empty()) continue;

            PlayerPtr target = sellingTeam->getSeniorSquad()[Utils::randInt(0, sellingTeam->getSeniorSquad().size() - 1)];
            
            // Logic Filters
            if (isProjectBuilder && target->age > 24) continue; // Only wants youth
            if (isDesperate && target->currentAbility < 130) continue; // Needs instant first-team impact
            if (target->reputation < buyingTeam->getLevel()) continue; // Reputation snob filter

            int64_t marketValue = calculateMarketValue(target, buyingTeam);
            int64_t bidAmount = marketValue;

            // Negotiation Sway (Behavioral framing)
            if (isDesperate) bidAmount = static_cast<int64_t>(marketValue * 1.3); // Panic overpay
            else bidAmount = static_cast<int64_t>(marketValue * 0.9); // Lowball opening

            if (buyingTeam->finances.transferBudget >= bidAmount) {
                TransferBid bid{buyingTeam, sellingTeam, target, bidAmount, false, 0, false};
                
                // Seller AI Logic
                int64_t acceptablePrice = marketValue;
                // If the seller is in debt, they accept lower offers to survive
                if (sellingTeam->finances.bankLoanDebt > 0) {
                    acceptablePrice = static_cast<int64_t>(marketValue * 0.75); 
                }
                
                if (bidAmount >= acceptablePrice) {
                    bid.accepted = true;
                    completeTransfer(bid);
                }
            }
        }
    }
}

bool TransferEngine::completeTransfer(TransferBid& bid) {
    if (!bid.accepted || !bid.buyer || !bid.seller || !bid.player) return false;
    if (bid.buyer->finances.transferBudget < bid.amount) return false;

    // Financial exchange
    bid.seller->finances.balance += bid.amount;
    bid.seller->finances.transferBudget += static_cast<int64_t>(bid.amount * 0.5); // Board allocates 50% to transfer budget
    
    bid.buyer->finances.balance -= bid.amount;
    bid.buyer->finances.transferBudget -= bid.amount;

    // Contract generation
    int64_t newWage = static_cast<int64_t>(bid.player->wage * 1.2); // AI gives a standard 20% bump
    bid.buyer->finances.wageBudget -= newWage;
    bid.seller->finances.wageBudget += bid.player->wage;

    bid.player->wage = newWage;
    bid.player->contractLengthMonths = 36; // Standard 3 year deal

    // Squad array movement
    bid.seller->removePlayer(bid.player->uniqueId);
    bid.buyer->addPlayer(bid.player, false);

    return true;
}
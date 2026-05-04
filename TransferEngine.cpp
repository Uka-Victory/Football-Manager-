// TransferEngine.cpp
#include "TransferEngine.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cmath>

int64_t TransferEngine::calculateMarketValue(const PlayerPtr& p, const TeamPtr& buyingTeam) {
    // Base: sum of all visible attributes scaled to 1-20 average
    double avgAttr = p->getOverall();
    double base = avgAttr * 500000.0;  // £50k per attribute point as base

    double ageFactor = getAgeFactor(p->getAge());
    double contractFactor = getContractFactor(p->getContractLengthMonths());
    double formFactor = getFormFactor(p);
    double homeGrownFactor = getHomeGrownFactor(p, buyingTeam);
    double playstyleMult = getPlaystyleMultiplier(p->getPlaystyle());
    double traitMult = getTraitMultiplier(p->getTraits());

    double value = base * ageFactor * contractFactor * formFactor * homeGrownFactor 
                   * playstyleMult * traitMult;
    
    // Clamp to reasonable range
    if (value < 10000) value = 10000;
    if (value > 500000000) value = 500000000;
    
    return static_cast<int64_t>(value);
}

int TransferEngine::calculateWeeklyWage(const PlayerPtr& p) {
    int64_t value = calculateMarketValue(p);
    int wage = static_cast<int>(value * 0.0005);  // ~2.6% annually
    if (wage < 500) wage = 500;
    if (wage > 500000) wage = 500000;
    return wage;
}

double TransferEngine::getAgeFactor(int age) {
    if (age <= 20) return 1.4;
    if (age <= 25) return 1.6;
    if (age <= 29) return 1.3;
    if (age <= 32) return 0.9;
    if (age <= 34) return 0.6;
    return 0.3;
}

double TransferEngine::getContractFactor(int months) {
    if (months < 6)  return 0.4;
    if (months < 12) return 0.7;
    if (months < 24) return 0.9;
    if (months < 36) return 1.0;
    if (months < 48) return 1.2;
    return 1.4;
}

double TransferEngine::getFormFactor(const PlayerPtr& p) {
    const auto& ratings = p->getLast5Ratings();
    if (ratings.empty()) return 1.0;
    double avg = 0.0;
    for (double r : ratings) avg += r;
    avg /= ratings.size();
    if (avg < 6.0) return 0.8;
    if (avg < 7.0) return 0.9;
    if (avg < 8.0) return 1.0;
    if (avg < 9.0) return 1.2;
    return 1.5;
}

double TransferEngine::getHomeGrownFactor(const PlayerPtr& p, const TeamPtr& buyer) {
    if (!buyer) return 1.0;
    if (p->getHomeGrownNation() == buyer->getCountry()) return 1.3;
    return 1.0;
}

double TransferEngine::getPlaystyleMultiplier(Playstyle ps) {
    switch (ps) {
        case Playstyle::DeepLyingPlaymaker:
        case Playstyle::AdvancedPlaymaker:
        case Playstyle::Raumdeuter:
            return 1.15;
        case Playstyle::Poacher:
        case Playstyle::InsideForward:
            return 1.10;
        default:
            return 1.0;
    }
}

double TransferEngine::getTraitMultiplier(const std::vector<Trait>& traits) {
    double mult = 1.0;
    for (auto t : traits) {
        switch (t) {
            case Trait::InjuryProne:     mult *= 0.75; break;
            case Trait::Consistent:       mult *= 1.10; break;
            case Trait::Inconsistent:     mult *= 0.85; break;
            case Trait::OneClubMan:      mult *= 1.05; break;
            case Trait::BigMatchPlayer:   mult *= 1.15; break;
            case Trait::PenaltySpecialist: mult *= 1.05; break;
            case Trait::SetPieceSpecialist:mult *= 1.05; break;
            default: break;
        }
    }
    return mult;
}

bool TransferEngine::isTransferWindowOpen(int month) {
    return (month == 1 || month == 7 || month == 8);
}

void TransferEngine::processAITransfers(std::vector<TeamPtr>& allTeams,
                                        std::vector<TransferBid>& activeBids,
                                        int currentYear, int currentMonth) {
    if (!isTransferWindowOpen(currentMonth)) return;
    if (allTeams.size() < 2) return;

    // Each AI team considers 1-2 transfer targets
    for (auto& buyer : allTeams) {
        if (buyer->getSeniorSquad().size() >= 35) continue;  // squad full
        if (Utils::randInt(1, 100) > 40) continue;  // 40% chance to be active

        // Find weakest position
        std::string neededPos = "";
        int minDepth = 99;
        const std::vector<std::string> positions = {
            "GK","CB","LB","RB","DM","CM","LM","RM","AM","LW","RW","ST"
        };
        for (const auto& pos : positions) {
            int count = 0;
            for (auto& p : buyer->getSeniorSquad()) {
                if (p->getPrimaryPosition() == pos) count++;
            }
            if (count < minDepth && count < 2) {
                minDepth = count;
                neededPos = pos;
            }
        }

        // Find a selling club with a player in that position
        for (auto& seller : allTeams) {
            if (seller == buyer) continue;
            for (auto& p : seller->getSeniorSquad()) {
                if (p->getPrimaryPosition() != neededPos) continue;
                if (p->getAge() > 34) continue;
                
                int64_t value = calculateMarketValue(p, buyer);
                int64_t bidAmount = static_cast<int64_t>(value * (0.7 + Utils::randDouble() * 0.2));
                
                // Check buyer budget (placeholder check)
                if (bidAmount > 50000000) continue;  // rough budget limit
                
                TransferBid bid;
                bid.buyer = buyer;
                bid.seller = seller;
                bid.player = p;
                bid.amount = bidAmount;
                bid.accepted = false;
                bid.rejected = false;
                activeBids.push_back(bid);
                break;
            }
            if (!activeBids.empty() && activeBids.back().player) break;
        }
    }
}

void TransferEngine::completeTransfer(TransferBid& bid) {
    if (!bid.accepted || bid.rejected) return;
    
    TeamPtr seller = bid.seller;
    TeamPtr buyer = bid.buyer;
    PlayerPtr player = bid.player;
    
    if (!seller || !buyer || !player) return;
    
    // Remove from seller
    seller->removePlayer(player->getUniqueId());
    
    // Set new contract
    player->setContractLengthMonths(24 + Utils::randInt(0, 24));
    player->setWeeklyWage(calculateWeeklyWage(player));
    
    // Add to buyer
    buyer->addToSenior(player);
    
    // Update records
    buyer->updateTransferRecord(bid.amount, false);
    seller->updateTransferRecord(bid.amount, true);
}

void TransferEngine::negotiateBid(TransferBid& bid, int64_t counterOffer) {
    if (counterOffer <= bid.amount * 1.3) {
        bid.amount = counterOffer;
        bid.accepted = true;
    } else {
        bid.rejected = true;
    }
}
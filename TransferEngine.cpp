#include "TransferEngine.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <sstream>

void TransferEngine::listPlayerForTransfer(Player* p, const std::string& club, int price, const std::string& date, int wage) {
    TransferListing listing;
    listing.playerId = p->uniqueId;
    listing.playerName = p->name;
    listing.currentClub = club;
    listing.age = p->age;
    listing.position = p->position;
    listing.overall = p->overall;
    listing.potential = p->potential;
    listing.askingPrice = price;
    listing.listedDate = date;
    listing.transferStatus = "Transfer Listed";
    listing.weeklyWage = wage;
    listing.contractEnd = "2028-06-30";
    listings[listing.playerId] = listing;
    p->transferStatus = "Transfer Listed";
    p->askingPrice = price;
}

void TransferEngine::removeListing(const std::string& playerId) {
    listings.erase(playerId);
}

std::vector<TransferListing> TransferEngine::getAvailableListings() const {
    std::vector<TransferListing> result;
    for (const auto& p : listings) result.push_back(p.second);
    return result;
}

void TransferEngine::submitBid(const std::string& playerId, const std::string& buyer, int amount, int wage, const std::string& date) {
    if (listings.count(playerId) == 0) return;
    TransferBid bid;
    bid.bidId = playerId + "_" + buyer + "_" + date;
    bid.playerId = playerId;
    bid.playerName = listings[playerId].playerName;
    bid.sellingClub = listings[playerId].currentClub;
    bid.buyingClub = buyer;
    bid.bidAmount = amount;
    bid.bidDate = date;
    bid.status = "Pending";
    bid.wageOffer = wage;
    bid.contractYears = 3;
    bid.responseDeadline = date;
    bids.push_back(bid);
}

void TransferEngine::processAIActivity(std::vector<Team>& teams, const std::string& currentDate) {
    for (auto& bid : bids) {
        if (bid.status == "Pending") {
            auto it = listings.find(bid.playerId);
            if (it != listings.end()) {
                if (bid.bidAmount >= it->second.askingPrice && randDouble(0,1) < 0.5) {
                    bid.status = "Accepted";
                } else if (randDouble(0,1) < 0.2) {
                    bid.status = "Rejected";
                }
            }
        }
    }
    
    for (const auto& listing : getAvailableListings()) {
        if (listing.transferStatus != "Transfer Listed") continue;
        for (auto& team : teams) {
            if (team.name == listing.currentClub) continue;
            if (team.finances.transferBudget < static_cast<int>(listing.askingPrice * 0.8)) continue;
            if (randDouble(0,1) < 0.03) {
                int wage = std::min(static_cast<int>(listing.weeklyWage * randDouble(0.9, 1.2)), team.finances.wageBudget / 2);
                submitBid(listing.playerId, team.name, static_cast<int>(listing.askingPrice * randDouble(0.85, 1.1)), wage, currentDate);
            }
        }
    }
    
    for (auto& bid : bids) {
        if (bid.status == "Accepted") {
            completeTransfer(bid, teams);
        }
    }
    bids.erase(std::remove_if(bids.begin(), bids.end(), [](const TransferBid& b) {
        return b.status == "Accepted" || b.status == "Rejected";
    }), bids.end());
}

void TransferEngine::completeTransfer(TransferBid& bid, std::vector<Team>& teams) {
    Team* seller = nullptr;
    Team* buyer = nullptr;
    for (auto& t : teams) {
        if (t.name == bid.sellingClub) seller = &t;
        if (t.name == bid.buyingClub) buyer = &t;
    }
    if (!seller || !buyer) return;
    
    Player* player = nullptr;
    bool fromYouth = false;
    for (auto& p : seller->players) {
        if (p.uniqueId == bid.playerId) {
            player = &p;
            break;
        }
    }
    if (!player) {
        for (auto& p : seller->youthPlayers) {
            if (p.uniqueId == bid.playerId) {
                player = &p;
                fromYouth = true;
                break;
            }
        }
    }
    if (!player) return;
    
    if (buyer->finances.transferBudget < bid.bidAmount || buyer->finances.balance < bid.bidAmount) return;
    
    if (fromYouth) {
        seller->youthPlayers.erase(std::remove_if(seller->youthPlayers.begin(), seller->youthPlayers.end(),
            [&](const Player& p) { return &p == player; }), seller->youthPlayers.end());
    } else {
        seller->players.erase(std::remove_if(seller->players.begin(), seller->players.end(),
            [&](const Player& p) { return &p == player; }), seller->players.end());
    }
    buyer->players.push_back(*player);
    
    seller->finances.balance += bid.bidAmount;
    seller->finances.transferBudget += static_cast<int>(bid.bidAmount * 0.8);
    buyer->finances.balance -= bid.bidAmount;
    buyer->finances.transferBudget -= bid.bidAmount;
    
    buyer->finances.contracts[bid.playerId] = {bid.playerId, player->name, bid.wageOffer, "2028-06-30", false};
    
    player->transferStatus = "Unavailable";
    player->askingPrice = 0;
    
    removeListing(bid.playerId);
    history.push_back(bid.bidDate + ": " + player->name + " from " + seller->name + " to " + buyer->name + " for $" + std::to_string(bid.bidAmount));
}
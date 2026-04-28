#ifndef TRANSFERENGINE_HPP
#define TRANSFERENGINE_HPP

#include <string>
#include <vector>
#include <map>
#include "Player.hpp"
#include "Team.hpp"

struct TransferListing {
    std::string playerId;
    std::string playerName;
    std::string currentClub;
    int age;
    std::string position;
    int overall;
    int potential;
    int askingPrice;
    std::string listedDate;
    std::string transferStatus;
    int weeklyWage;
    std::string contractEnd;
};

struct TransferBid {
    std::string bidId;
    std::string playerId;
    std::string playerName;
    std::string sellingClub;
    std::string buyingClub;
    int bidAmount;
    std::string bidDate;
    std::string status;
    int wageOffer;
    int contractYears;
    std::string responseDeadline;
};

class TransferEngine {
public:
    std::map<std::string, TransferListing> listings;
    std::vector<TransferBid> bids;
    std::vector<std::string> history;
    
    void listPlayerForTransfer(Player* p, const std::string& club, int price, const std::string& date, int wage);
    void removeListing(const std::string& playerId);
    std::vector<TransferListing> getAvailableListings() const;
    void submitBid(const std::string& playerId, const std::string& buyer, int amount, int wage, const std::string& date);
    void processAIActivity(std::vector<Team>& teams, const std::string& currentDate);
    void completeTransfer(TransferBid& bid, std::vector<Team>& teams);
};

#endif
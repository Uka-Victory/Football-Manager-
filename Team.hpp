#ifndef TEAM_HPP
#define TEAM_HPP

#include <string>
#include <vector>
#include <memory>
#include "json.hpp"
#include "Player.hpp"
#include "Facilities.hpp" // NEW: Include Facilities

class Team; 
using TeamPtr = std::shared_ptr<Team>;

class Team {
private:
    std::string name;
    int level;
    int64_t balance;
    int64_t transferBudget;
    int64_t wageBudget;
    std::vector<PlayerPtr> squad;
    
    Facilities facilities; // NEW: Every team owns a Facilities object

public:
    Team();
    Team(std::string teamName, int teamLevel);

    // Getters
    std::string getName() const;
    int getLevel() const;
    int getTeamOverall() const;
    
    // Financials
    int64_t getBalance() const;
    int64_t getTransferBudget() const;
    int64_t getWageBudget() const;
    void addFunds(int64_t amount);
    void deductFunds(int64_t amount);
    void setBudgets(int64_t transfer, int64_t wage);

    // Squad Management
    void addPlayer(PlayerPtr player);
    void removePlayer(const std::string& playerId);
    PlayerPtr getPlayerById(const std::string& playerId) const;
    const std::vector<PlayerPtr>& getPlayers() const;

    // NEW: Facilities Access
    Facilities& getFacilities();

    // Save/Load
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

#endif
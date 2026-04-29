#ifndef TEAM_HPP
#define TEAM_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include "Player.hpp"
#include "json.hpp"

class Team {
private:
    // Core Data
    std::string name;
    int level; // League tier
    
    // Finances (Upgraded to int64_t)
    int64_t balance;
    int64_t transferBudget;
    int64_t wageBudget;

    // Player Roster
    std::vector<PlayerPtr> players; // Useful for displaying the squad in order
    std::unordered_map<std::string, PlayerPtr> playerRegistry; // O(1) instant lookups by ID

public:
    // Constructors
    Team() = default;
    Team(std::string teamName, int teamLevel);

    // Getters & Setters
    std::string getName() const;
    int getLevel() const;
    
    // Financial Methods
    int64_t getBalance() const;
    int64_t getTransferBudget() const;
    int64_t getWageBudget() const;
    void addFunds(int64_t amount);
    bool deductFunds(int64_t amount);
    void setBudgets(int64_t transfer, int64_t wage);

    // Player Management (Using Smart Pointers)
    void addPlayer(PlayerPtr player);
    void removePlayer(const std::string& playerId);
    PlayerPtr getPlayerById(const std::string& playerId) const;
    const std::vector<PlayerPtr>& getPlayers() const;

    // Tactical/Team Stats
    int getTeamOverall() const;
    
    // Save/Load System Methods
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

// Smart pointer typedef for Team, just like we did for Player
using TeamPtr = std::shared_ptr<Team>;

#endif
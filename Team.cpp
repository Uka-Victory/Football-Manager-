l#include "Team.hpp"
#include <algorithm>

Team::Team(std::string teamName, int teamLevel)
    : name(teamName), level(teamLevel), balance(0), transferBudget(0), wageBudget(0) {}

std::string Team::getName() const { return name; }
int Team::getLevel() const { return level; }

int64_t Team::getBalance() const { return balance; }
int64_t Team::getTransferBudget() const { return transferBudget; }
int64_t Team::getWageBudget() const { return wageBudget; }

void Team::addFunds(int64_t amount) { 
    balance += amount; 
}

bool Team::deductFunds(int64_t amount) {
    if (balance >= amount) {
        balance -= amount;
        return true;
    }
    return false;
}

void Team::setBudgets(int64_t transfer, int64_t wage) {
    transferBudget = transfer;
    wageBudget = wage;
}

// --- PLAYER MANAGEMENT (Smart Pointers & Hash Map) ---

void Team::addPlayer(PlayerPtr player) {
    if (!player) return; // Safety check
    players.push_back(player); // Add to display roster
    playerRegistry[player->uniqueId] = player; // Add to O(1) lookup map
}

void Team::removePlayer(const std::string& playerId) {
    // 1. Remove from the instant lookup registry
    playerRegistry.erase(playerId);
    
    // 2. Safely remove from the roster vector
    players.erase(std::remove_if(players.begin(), players.end(),
        [&playerId](const PlayerPtr& p) { return p->uniqueId == playerId; }), 
        players.end());
}

PlayerPtr Team::getPlayerById(const std::string& playerId) const {
    // O(1) Instant Hash Map Lookup!
    auto it = playerRegistry.find(playerId);
    if (it != playerRegistry.end()) {
        return it->second; // Found them, return the pointer
    }
    return nullptr; // Player not found
}

const std::vector<PlayerPtr>& Team::getPlayers() const {
    return players;
}

int Team::getTeamOverall() const {
    if (players.empty()) return 0;
    int total = 0;
    for (const auto& p : players) {
        total += p->overall;
    }
    return total / players.size();
}

// --- SAVE / LOAD SYSTEM ---

nlohmann::json Team::toJson() const {
    nlohmann::json j;
    j["name"] = name;
    j["level"] = level;
    j["balance"] = balance;
    j["transferBudget"] = transferBudget;
    j["wageBudget"] = wageBudget;
    
    // Serialize every player into a JSON array
    nlohmann::json playersJson = nlohmann::json::array();
    for (const auto& p : players) {
        playersJson.push_back(p->toJson());
    }
    j["players"] = playersJson;
    
    return j;
}

void Team::fromJson(const nlohmann::json& j) {
    name = j.value("name", "Unknown Team");
    level = j.value("level", 1);
    balance = j.value("balance", 0LL);
    transferBudget = j.value("transferBudget", 0LL);
    wageBudget = j.value("wageBudget", 0LL);
    
    // Clear out existing players before loading
    players.clear();
    playerRegistry.clear();
    
    if (j.contains("players")) {
        for (const auto& pJson : j["players"]) {
            // Create a new smart pointer for each loaded player
            auto p = std::make_shared<Player>();
            p->fromJson(pJson);
            addPlayer(p); // This adds them to both the vector and the hash map
        }
    }
}
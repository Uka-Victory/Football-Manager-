#include "Team.hpp"
#include <numeric>

Team::Team() : name("Unknown"), level(1), balance(0), transferBudget(0), wageBudget(0) {}

Team::Team(std::string teamName, int teamLevel) 
    : name(teamName), level(teamLevel), balance(0), transferBudget(0), wageBudget(0) {}

std::string Team::getName() const { return name; }
int Team::getLevel() const { return level; }

int64_t Team::getBalance() const { return balance; }
int64_t Team::getTransferBudget() const { return transferBudget; }
int64_t Team::getWageBudget() const { return wageBudget; }

void Team::addFunds(int64_t amount) { balance += amount; }
void Team::deductFunds(int64_t amount) { balance -= amount; if (balance < 0) balance = 0; }

void Team::setBudgets(int64_t transfer, int64_t wage) {
    transferBudget = transfer;
    wageBudget = wage;
}

void Team::addPlayer(PlayerPtr player) {
    if (player) squad.push_back(player);
}

void Team::removePlayer(const std::string& playerId) {
    auto it = std::remove_if(squad.begin(), squad.end(), [&](const PlayerPtr& p) {
        return p->id == playerId;
    });
    if (it != squad.end()) {
        squad.erase(it, squad.end());
    }
}

PlayerPtr Team::getPlayerById(const std::string& playerId) const {
    for (const auto& p : squad) {
        if (p->id == playerId) return p;
    }
    return nullptr;
}

const std::vector<PlayerPtr>& Team::getPlayers() const {
    return squad;
}

int Team::getTeamOverall() const {
    if (squad.empty()) return 0;
    int totalOvr = 0;
    for (const auto& p : squad) {
        totalOvr += p->overall;
    }
    return totalOvr / squad.size();
}

// NEW: Facilities Getter
Facilities& Team::getFacilities() {
    return facilities;
}

// --- SAVE / LOAD ---

nlohmann::json Team::toJson() const {
    nlohmann::json j;
    j["name"] = name;
    j["level"] = level;
    j["balance"] = balance;
    j["transferBudget"] = transferBudget;
    j["wageBudget"] = wageBudget;

    nlohmann::json playersJson = nlohmann::json::array();
    for (const auto& p : squad) {
        playersJson.push_back(p->toJson());
    }
    j["squad"] = playersJson;

    // NEW: Save Facilities
    j["facilities"] = facilities.toJson();

    return j;
}

void Team::fromJson(const nlohmann::json& j) {
    name = j.value("name", "Unknown");
    level = j.value("level", 1);
    balance = j.value("balance", 0LL);
    transferBudget = j.value("transferBudget", 0LL);
    wageBudget = j.value("wageBudget", 0LL);

    squad.clear();
    if (j.contains("squad")) {
        for (const auto& playerJson : j["squad"]) {
            auto p = std::make_shared<Player>();
            p->fromJson(playerJson);
            squad.push_back(p);
        }
    }

    // NEW: Load Facilities
    if (j.contains("facilities")) {
        facilities.fromJson(j["facilities"]);
    }
}
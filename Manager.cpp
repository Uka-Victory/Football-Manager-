#include "Manager.hpp"
#include <algorithm>

Manager::Manager(const std::string& n, int a, int rep) 
    : name(n), age(a), reputation(std::clamp(rep, 1, 20)) {}

void Manager::recordMatch(bool isWin, bool isDraw) {
    if (isWin) {
        wins++;
    } else if (isDraw) {
        draws++;
    } else {
        losses++;
    }
}

void Manager::addTrophy(const std::string& trophyName) {
    trophiesWon++;
    trophyList.push_back(trophyName);
    // Boost reputation slightly when winning trophies, capped at 20
    if (reputation < 20) reputation++;
}

nlohmann::json Manager::toJson() const {
    return {
        {"name", name}, {"age", age}, {"rep", reputation}, 
        {"w", wins}, {"d", draws}, {"l", losses}, 
        {"club", currentClubName}, {"trophies", trophyList}
    };
}

void Manager::fromJson(const nlohmann::json& j) {
    name = j.value("name", "Unknown"); 
    age = j.value("age", 40);
    reputation = j.value("rep", 10); 
    wins = j.value("w", 0);
    draws = j.value("d", 0); 
    losses = j.value("l", 0);
    currentClubName = j.value("club", "");
    
    trophyList.clear();
    if (j.contains("trophies")) {
        for (const auto& t : j["trophies"]) {
            trophyList.push_back(t.get<std::string>());
        }
        trophiesWon = static_cast<int>(trophyList.size());
    }
}
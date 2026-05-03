#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <string>
#include <vector>
#include <memory>
#include "json.hpp"

class Manager {
public:
    std::string name;
    int age = 40;
    int reputation = 10;   // 1-20 scale
    
    int wins = 0;
    int draws = 0;
    int losses = 0;
    int trophiesWon = 0;
    
    std::string currentClubName;
    std::vector<std::string> trophyList;

    Manager() = default;
    Manager(const std::string& n, int a, int rep);

    void recordMatch(bool isWin, bool isDraw);
    void addTrophy(const std::string& trophyName);

    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

using ManagerPtr = std::shared_ptr<Manager>;

#endif
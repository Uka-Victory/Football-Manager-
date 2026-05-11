// Manager.hpp
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "json.hpp"
using json = nlohmann::json;

class Manager {
public:
    Manager(const std::string& n = "", int a = 40, int rep = 50);
    void setCurrentClubName(const std::string& name);
    std::string getCurrentClubName() const;
    int getReputation() const { return reputation; }
    void setReputation(int r) { reputation = r; }
    int getWins() const { return wins; }
    int getDraws() const { return draws; }
    int getLosses() const { return losses; }
    void recordResult(bool win, bool draw) {
        if (win) wins++;
        else if (draw) draws++;
        else losses++;
    }
    void addTrophy(const std::string& t) { trophyList.push_back(t); }
    const std::vector<std::string>& getTrophies() const { return trophyList; }
    const std::string& getName() const { return name; }
    json toJson() const;
    void fromJson(const json& j);
private:
    std::string name;
    int age = 40;
    int reputation = 50;
    std::string currentClub;
    int wins = 0, draws = 0, losses = 0;
    std::vector<std::string> trophyList;
};

using ManagerPtr = std::shared_ptr<Manager>;

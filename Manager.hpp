// Manager.hpp
#pragma once
#include <string>
#include <vector>
#include "json.hpp"
using json = nlohmann::json;

class Manager {
public:
    Manager(const std::string& n = "", int a = 40, int rep = 50);
    void setCurrentClubName(const std::string& name);
    std::string getCurrentClubName() const;
    json toJson() const;
    void fromJson(const json& j);
private:
    std::string name;
    int age, reputation;
    std::string currentClub;
    int wins = 0, draws = 0, losses = 0;
    std::vector<std::string> trophyList;
};
// Manager.cpp
#include "Manager.hpp"

Manager::Manager(const std::string& n, int a, int rep) : name(n), age(a), reputation(rep) {}
void Manager::setCurrentClubName(const std::string& club) { currentClub = club; }
std::string Manager::getCurrentClubName() const { return currentClub; }

json Manager::toJson() const {
    return {{"name", name}, {"age", age}, {"reputation", reputation},
            {"currentClub", currentClub}, {"trophies", trophyList}};
}

void Manager::fromJson(const json& j) {
    name = j.value("name", "Unknown");
    age = j.value("age", 40);
    reputation = j.value("reputation", 50);
    currentClub = j.value("currentClub", "");
}
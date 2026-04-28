#include "League.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

League::League(std::string n) : name(n) {}

void League::addTeam(Team* t) {
    teams.push_back(t);
}

std::vector<Team*> League::getSortedTable() const {
    std::vector<Team*> sorted = teams;
    std::sort(sorted.begin(), sorted.end(), [](Team* a, Team* b) {
        if (a->points != b->points) return a->points > b->points;
        if (a->getGoalDifference() != b->getGoalDifference())
            return a->getGoalDifference() > b->getGoalDifference();
        return a->goalsFor > b->goalsFor;
    });
    return sorted;
}

void League::printTable() const {
    auto sorted = getSortedTable();
    std::cout << "\n=== " << name << " TABLE ===\n";
    std::cout << std::left << std::setw(4) << "Pos" << std::setw(20) << "Team"
              << std::setw(3) << "P" << std::setw(3) << "W" << std::setw(3) << "D"
              << std::setw(3) << "L" << std::setw(4) << "GF" << std::setw(4) << "GA"
              << std::setw(4) << "GD" << std::setw(4) << "Pts\n";
    int pos = 1;
    for (auto t : sorted) {
        std::cout << std::left << std::setw(4) << pos++ << std::setw(20) << t->name
                  << std::setw(3) << t->gamesPlayed << std::setw(3) << t->wins
                  << std::setw(3) << t->draws << std::setw(3) << t->losses
                  << std::setw(4) << t->goalsFor << std::setw(4) << t->goalsAgainst
                  << std::setw(4) << t->getGoalDifference() << std::setw(4) << t->points << "\n";
    }
}
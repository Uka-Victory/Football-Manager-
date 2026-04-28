#ifndef LEAGUE_HPP
#define LEAGUE_HPP

#include <string>
#include <vector>
#include "Team.hpp"

class League {
public:
    std::string name;
    std::vector<Team*> teams;
    
    League(std::string n);
    void addTeam(Team* t);
    std::vector<Team*> getSortedTable() const;
    void printTable() const;
};

#endif
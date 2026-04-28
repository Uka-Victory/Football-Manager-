#ifndef TEAM_HPP
#define TEAM_HPP

#include <string>
#include <vector>
#include "Player.hpp"
#include "Finances.hpp"

class Team {
public:
    std::string name;
    std::string country;
    int level;                     // NEW: 1-20 team strength level
    std::vector<Player> players;
    std::vector<Player> youthPlayers;
    std::string youthTeamType;
    
    ClubFinances finances;
    
    int gamesPlayed;
    int wins;
    int draws;
    int losses;
    int goalsFor;
    int goalsAgainst;
    int points;
    
    Team(std::string n, std::string c, int lvl, int initialBalance);
    void addPlayer(const Player& p);
    void addYouthPlayer(const Player& p);
    Player* findPlayer(const std::string& playerName);
    void updateRecord(int gf, int ga);
    int getGoalDifference() const;
};

#endif
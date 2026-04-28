#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>

class Player {
public:
    std::string name;
    int age;
    std::string nationality;
    std::string position;
    int overall;
    int potential;
    
    int appearances;
    int goals;
    int assists;
    int cleanSheets;
    int yellowCards;
    int redCards;
    float averageRating;
    
    int morale;
    int fitness;
    
    std::string transferStatus;
    int askingPrice;

    std::string uniqueId;   // unique identifier for transfers
    
    Player(std::string n, int a, std::string nat, std::string pos, int ovr, int pot);
    std::string shortInfo() const;
    void updateMatchStats(int goalsScored, int assistsMade, bool cleanSheet, float rating);
};

#endif
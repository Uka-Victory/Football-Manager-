#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>
#include <memory>
#include <cstdint>
#include "json.hpp"

class Player {
public:
    std::string uniqueId; // Now crucial for O(1) hash map lookups
    std::string name;
    int age;
    std::string nationality;
    std::string position;
    std::string positionRole;
    
    int overall;
    int potential;
    
    // Match Stats
    int appearances;
    int goals;
    int assists;
    int cleanSheets;
    int yellowCards;
    int redCards;
    float averageRating;
    
    // Dynamic Status (The "Living World" update)
    int morale;    // 0-100
    int fitness;   // 0-100 (100 is fully fit)
    int form;      // 0-100
    
    // Finances (Upgraded to int64_t to prevent overflow with big money)
    std::string transferStatus;
    int64_t askingPrice; 
    int64_t wage;
    
    // Constructors
    Player() = default; // Required for JSON loading
    Player(std::string id, std::string n, int a, std::string nat, std::string pos, std::string role, int ovr, int pot);
    
    // Core Functions
    std::string shortInfo() const;
    void updateMatchStats(int goalsScored, int assistsMade, bool cleanSheet, int cards, float rating);
    void applyFatigue(int minutesPlayed);
    void recoverFitness(int restDays);
    
    // Save/Load System Methods
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

// This is the magic line. Instead of passing massive Player objects around,
// we will just pass a 'PlayerPtr'. It's incredibly fast and prevents memory leaks.
using PlayerPtr = std::shared_ptr<Player>;

#endif
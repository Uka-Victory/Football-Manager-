#include "Player.hpp"
#include <sstream>

// Simple counter for unique IDs (you can replace with UUID if desired)
static int nextPlayerId = 1000;

Player::Player(std::string n, int a, std::string nat, std::string pos, int ovr, int pot)
    : name(n), age(a), nationality(nat), position(pos), overall(ovr), potential(pot),
      appearances(0), goals(0), assists(0), cleanSheets(0), yellowCards(0), redCards(0),
      averageRating(0.0f), morale(70), fitness(100),
      transferStatus("Unavailable"), askingPrice(0) {
    // Generate unique ID
    std::ostringstream oss;
    oss << "player_" << nextPlayerId++;
    uniqueId = oss.str();
}

std::string Player::shortInfo() const {
    std::ostringstream oss;
    oss << name << " | " << position << " | OVR " << overall 
        << " | POT " << potential << " | " << nationality;
    return oss.str();
}

void Player::updateMatchStats(int goalsScored, int assistsMade, bool cleanSheet, float rating) {
    appearances++;
    goals += goalsScored;
    assists += assistsMade;
    if (cleanSheet) cleanSheets++;
    if (appearances == 1) averageRating = rating;
    else averageRating = ((averageRating * (appearances - 1)) + rating) / appearances;
}
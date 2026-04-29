#include "Player.hpp"
#include <sstream>
#include <algorithm>

// Constructor with our new variables initialized
Player::Player(std::string id, std::string n, int a, std::string nat, std::string pos, std::string role, int ovr, int pot)
    : uniqueId(id), name(n), age(a), nationality(nat), position(pos), positionRole(role),
      overall(ovr), potential(pot), appearances(0), goals(0), assists(0),
      cleanSheets(0), yellowCards(0), redCards(0), averageRating(0.0f),
      morale(80), fitness(100), form(50), transferStatus("Not Listed"), askingPrice(0), wage(0) {}

std::string Player::shortInfo() const {
    std::stringstream ss;
    ss << name << " (" << position << ", " << overall << " OVR, " << age << "yo)";
    return ss.str();
}

void Player::updateMatchStats(int goalsScored, int assistsMade, bool cleanSheet, int cards, float rating) {
    appearances++;
    goals += goalsScored;
    assists += assistsMade;
    if (cleanSheet) cleanSheets++;
    
    // Simple card tracker: 1 = Yellow, 2 = Red
    if (cards == 1) yellowCards++;
    else if (cards >= 2) redCards++; 

    // Recalculate average rating
    if (appearances == 1) {
        averageRating = rating;
    } else {
        averageRating = ((averageRating * (appearances - 1)) + rating) / appearances;
    }
}

void Player::applyFatigue(int minutesPlayed) {
    // Rough estimate: drop fitness based on minutes played. 90 mins drops it by ~30.
    int fatigueDrop = minutesPlayed / 3; 
    fitness -= fatigueDrop;
    if (fitness < 0) fitness = 0;
}

void Player::recoverFitness(int restDays) {
    // Recover roughly 10 fitness points per rest day
    fitness += restDays * 10;
    if (fitness > 100) fitness = 100;
}

// --- SAVE / LOAD SYSTEM ---

nlohmann::json Player::toJson() const {
    return {
        {"uniqueId", uniqueId},
        {"name", name},
        {"age", age},
        {"nationality", nationality},
        {"position", position},
        {"positionRole", positionRole},
        {"overall", overall},
        {"potential", potential},
        {"appearances", appearances},
        {"goals", goals},
        {"assists", assists},
        {"cleanSheets", cleanSheets},
        {"yellowCards", yellowCards},
        {"redCards", redCards},
        {"averageRating", averageRating},
        {"morale", morale},
        {"fitness", fitness},
        {"form", form},
        {"transferStatus", transferStatus},
        {"askingPrice", askingPrice},
        {"wage", wage}
    };
}

void Player::fromJson(const nlohmann::json& j) {
    // Using j.value() allows us to set safe defaults just in case the JSON is missing a field
    uniqueId = j.value("uniqueId", "");
    name = j.value("name", "Unknown");
    age = j.value("age", 18);
    nationality = j.value("nationality", "Unknown");
    position = j.value("position", "SUB");
    positionRole = j.value("positionRole", "");
    overall = j.value("overall", 50);
    potential = j.value("potential", 50);
    
    appearances = j.value("appearances", 0);
    goals = j.value("goals", 0);
    assists = j.value("assists", 0);
    cleanSheets = j.value("cleanSheets", 0);
    yellowCards = j.value("yellowCards", 0);
    redCards = j.value("redCards", 0);
    averageRating = j.value("averageRating", 0.0f);
    
    morale = j.value("morale", 80);
    fitness = j.value("fitness", 100);
    form = j.value("form", 50);
    
    transferStatus = j.value("transferStatus", "Not Listed");
    askingPrice = j.value("askingPrice", 0LL); // 0LL ensures it reads as a 64-bit integer
    wage = j.value("wage", 0LL);
}
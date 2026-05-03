#ifndef TACTICS_HPP
#define TACTICS_HPP

#include <string>
#include <vector>

enum class Mentality { VeryDefensive, Defensive, Balanced, Attacking, VeryAttacking };
enum class PassingStyle { Short, Mixed, Direct };
enum class Tempo { Slow, Normal, Fast };
enum class PressingIntensity { Low, Medium, High, VeryHigh };

// Coordinates map directly to the 12x8 Match Engine grid
struct Coordinate {
    int x; // 1 to 12 (Length of the pitch)
    int y; // 1 to 8  (Width of the pitch)
};

class Tactics {
public:
    std::string formationName;
    std::vector<Coordinate> basePositions; // 11 grid coordinates

    Mentality mentality;
    PassingStyle passingStyle;
    Tempo tempo;
    PressingIntensity pressing;

    Tactics();
    
    // Sets the 11 coordinate offsets based on standard formations
    void setFormation(const std::string& formation);
    
    // Mathematical hooks for the Match Engine duel probabilities
    float getMentalityRiskMultiplier() const;
    float getStaminaDrainMultiplier() const;
};

#endif
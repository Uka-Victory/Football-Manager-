#ifndef FACILITIES_HPP
#define FACILITIES_HPP

#include "json.hpp"
#include <algorithm>

class Facilities {
private:
    int trainingLevel;  // Boosts daily attribute growth
    int academyLevel;   // Boosts youth intake quality/potential
    int stadiumLevel;   // Boosts matchday revenue
    int infirmaryLevel; // Boosts daily fitness recovery & prevents injuries

    const int MAX_LEVEL = 10;

public:
    Facilities();
    Facilities(int startTraining, int startAcademy, int startStadium, int startInfirmary);

    // Getters
    int getTrainingLevel() const;
    int getAcademyLevel() const;
    int getStadiumLevel() const;
    int getInfirmaryLevel() const;

    // Upgrades (Returns false if already at max level)
    bool upgradeTraining();
    bool upgradeAcademy();
    bool upgradeStadium();
    bool upgradeInfirmary();

    // Get Upgrade Costs (Scales exponentially with the current level)
    int64_t getUpgradeCost(int currentLevel) const;

    // Save/Load
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

#endif
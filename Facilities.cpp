#include "Facilities.hpp"
#include <cmath>

Facilities::Facilities() 
    : trainingLevel(1), academyLevel(1), stadiumLevel(1), infirmaryLevel(1) {}

Facilities::Facilities(int startTraining, int startAcademy, int startStadium, int startInfirmary)
    : trainingLevel(startTraining), academyLevel(startAcademy), stadiumLevel(startStadium), infirmaryLevel(startInfirmary) {}

int Facilities::getTrainingLevel() const { return trainingLevel; }
int Facilities::getAcademyLevel() const { return academyLevel; }
int Facilities::getStadiumLevel() const { return stadiumLevel; }
int Facilities::getInfirmaryLevel() const { return infirmaryLevel; }

int64_t Facilities::getUpgradeCost(int currentLevel) const {
    if (currentLevel >= MAX_LEVEL) return 0;
    // Base cost of £500k, scaling up to multi-millions for high levels
    return static_cast<int64_t>(500000 * std::pow(1.8, currentLevel));
}

bool Facilities::upgradeTraining() {
    if (trainingLevel < MAX_LEVEL) { trainingLevel++; return true; }
    return false;
}

bool Facilities::upgradeAcademy() {
    if (academyLevel < MAX_LEVEL) { academyLevel++; return true; }
    return false;
}

bool Facilities::upgradeStadium() {
    if (stadiumLevel < MAX_LEVEL) { stadiumLevel++; return true; }
    return false;
}

bool Facilities::upgradeInfirmary() {
    if (infirmaryLevel < MAX_LEVEL) { infirmaryLevel++; return true; }
    return false;
}

// --- SAVE / LOAD ---

nlohmann::json Facilities::toJson() const {
    return {
        {"training", trainingLevel},
        {"academy", academyLevel},
        {"stadium", stadiumLevel},
        {"infirmary", infirmaryLevel}
    };
}

void Facilities::fromJson(const nlohmann::json& j) {
    trainingLevel = j.value("training", 1);
    academyLevel = j.value("academy", 1);
    stadiumLevel = j.value("stadium", 1);
    infirmaryLevel = j.value("infirmary", 1);
}
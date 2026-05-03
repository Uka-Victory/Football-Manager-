#include "Facilities.hpp"
#include <algorithm>

namespace FootballManager {

    Facilities::Facilities(int startTraining, int startAcademy, int startStadium, int startInfirmary)
        : trainingLevel(std::clamp(startTraining, 1, 20)),
          academyLevel(std::clamp(startAcademy, 1, 20)),
          stadiumCapacity(startStadium),
          infirmaryLevel(std::clamp(startInfirmary, 1, 20)) {}

    float Facilities::getFatigueDecayModifier() const {
        // Higher infirmary level reduces fatigue decay speed
        return 1.0f - (infirmaryLevel * 0.02f); 
    }

    bool Facilities::upgradeTraining() {
        if (trainingLevel < 20) {
            trainingLevel++;
            return true;
        }
        return false;
    }

    bool Facilities::upgradeAcademy() {
        if (academyLevel < 20) {
            academyLevel++;
            return true;
        }
        return false;
    }

    bool Facilities::upgradeStadium() {
        stadiumCapacity += 5000;
        return true;
    }

    bool Facilities::upgradeInfirmary() {
        if (infirmaryLevel < 20) {
            infirmaryLevel++;
            return true;
        }
        return false;
    }

} // namespace FootballManager
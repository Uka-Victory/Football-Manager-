#pragma once

namespace FootballManager {

    class Facilities {
    private:
        int trainingLevel;  // 1-20
        int academyLevel;   // 1-20
        int stadiumCapacity;
        int infirmaryLevel; // 1-20

    public:
        Facilities(int startTraining, int startAcademy, int startStadium, int startInfirmary);

        // Functional Methods
        float getFatigueDecayModifier() const;
        bool upgradeTraining();
        bool upgradeAcademy();
        bool upgradeStadium();
        bool upgradeInfirmary();
        
        // Inline Getters
        int getTrainingLevel() const { return trainingLevel; }
        int getAcademyLevel() const { return academyLevel; }
        int getStadiumCapacity() const { return stadiumCapacity; }
        int getInfirmaryLevel() const { return infirmaryLevel; }
    };

} // namespace FootballManager
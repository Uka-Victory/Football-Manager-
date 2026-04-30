#include "TrainingEngine.hpp"
#include "Utils.hpp"

void TrainingEngine::processDailyTraining(TeamPtr team) {
    if (!team) return;

    // Dynamically fetch the team's actual facility levels
    int infirmaryLevel = team->getFacilities().getInfirmaryLevel(); 
    int trainingLevel = team->getFacilities().getTrainingLevel();  

    for (auto& player : team->getPlayers()) {
        
        // 1. FITNESS RECOVERY
        if (player->fitness < 100) {
            int recoveryAmount = Utils::randInt(2, 4) + (infirmaryLevel / 2);
            player->fitness += recoveryAmount;
            if (player->fitness > 100) player->fitness = 100;
        }

        // 2. ATTRIBUTE GROWTH (Micro-progression)
        if (player->age < 29 && player->overall < player->potential) {
            int growthChance = 10000 - (trainingLevel * 200); 
            if (Utils::randInt(1, 10000) > growthChance) {
                player->overall += 1;
                player->askingPrice += (player->askingPrice / 10);
                player->wage += (player->wage / 10);
            }
        }

        // 3. AGE DECLINE
        if (player->age >= 32 && player->overall > 40) {
            if (Utils::randInt(1, 20000) > 19900) { 
                player->overall -= 1;
                if (player->potential > player->overall) player->potential -= 1; 
            }
        }

        // 4. TRAINING INJURIES
        if (player->fitness < 70) {
            int injuryRisk = 5000 + (infirmaryLevel * 100);
            if (Utils::randInt(1, 10000) > injuryRisk) {
                // Injury logic will go here in the upcoming Injury System step
            }
        }
    }
}
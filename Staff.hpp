// Project: Football-Manager- (Uka-Victory)
#ifndef STAFF_HPP
#define STAFF_HPP

#include <string>
#include <memory>
#include "json.hpp"
#include "Player.hpp"

enum class StaffRole { HeadCoach, AssistantManager, GoalkeepingCoach, FitnessCoach, Physio, Scout, HeadOfYouthDevelopment };

class Staff {
public:
    std::string uniqueId;
    std::string name;
    StaffRole role;
    
    // Attributes (1-20 Scale)
    int attackingCoaching = 1;
    int defendingCoaching = 1;
    int fitnessCoaching = 1;
    int goalkeepingCoaching = 1;
    
    int physiotherapy = 1; // Reduces injury severity
    
    int judgingPlayerAbility = 1; // Shrinks the +/- variance on scout reports
    int judgingPlayerPotential = 1; // Determines accuracy of PA readings
    
    int workingWithYoungsters = 1; // Affects Youth Intake PA ceiling

    int64_t wage = 0;

    Staff() = default;
    Staff(const std::string& n, StaffRole r, int level);

    // Determines the max Potential Ability cap for regens during season rollover
    int getYouthIntakeModifier() const;
    
    // Returns the percentage of injury days to shave off
    float getPhysioRecoveryModifier() const;

    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

using StaffPtr = std::shared_ptr<Staff>;

#endif
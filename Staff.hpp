#pragma once
#include "Player.hpp"
#include <memory>
#include <string>

namespace FootballManager {

    enum class StaffRole { Manager, Assistant, Coach, Physio, Scout };

    class Staff {
    private:
        std::string name;
        StaffRole role;
        int attackingCoaching;
        int defendingCoaching;
        int fitnessCoaching;
        int physiotherapy;
        int judgingAbility;

    public:
        Staff(std::string staffName, StaffRole staffRole);
        
        StaffRole getRole() const { return role; }
        int getAttackingCoaching() const { return attackingCoaching; }
        int getDefendingCoaching() const { return defendingCoaching; }
        int getFitnessCoaching() const { return fitnessCoaching; }
        int getPhysiotherapy() const { return physiotherapy; }
        int getJudgingAbility() const { return judgingAbility; }
    };

} // namespace FootballManager
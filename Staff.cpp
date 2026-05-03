#include "Staff.hpp"

namespace FootballManager {

    Staff::Staff(std::string staffName, StaffRole staffRole) 
        : name(staffName), role(staffRole), 
          attackingCoaching(10), defendingCoaching(10), 
          fitnessCoaching(10), physiotherapy(10), judgingAbility(10) {
        
        // Base attributes default to 10 to avoid the "silent 1" logic bug
        switch(role) {
            case StaffRole::Coach: attackingCoaching = 15; defendingCoaching = 15; break;
            case StaffRole::Physio: physiotherapy = 18; break;
            case StaffRole::Scout: judgingAbility = 18; break;
            default: break;
        }
    }

} // namespace FootballManager
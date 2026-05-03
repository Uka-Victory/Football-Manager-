#include "Staff.hpp"
#include "Utils.hpp"
#include "Constants.hpp"
#include <algorithm>

Staff::Staff(const std::string& n, StaffRole r, int level) : name(n), role(r) {
    uniqueId = "S_" + Utils::generateUniqueId();
    
    // Level is the 1-20 club tier. Top tier clubs get top tier staff.
    auto genAttr = [&](int bonus) {
        int base = level + bonus + Utils::randInt(-3, 3);
        return std::clamp(base, FM::MIN_ATTR, FM::MAX_ATTR);
    };

    wage = static_cast<int64_t>(level) * 5000LL;

    switch (role) {
        case StaffRole::HeadCoach:
        case StaffRole::AssistantManager:
            attackingCoaching = genAttr(2);
            defendingCoaching = genAttr(2);
            break;
        case StaffRole::FitnessCoach:
            fitnessCoaching = genAttr(5);
            break;
        case StaffRole::GoalkeepingCoach:
            goalkeepingCoaching = genAttr(5);
            break;
        case StaffRole::Physio:
            physiotherapy = genAttr(5);
            break;
        case StaffRole::Scout:
            judgingPlayerAbility = genAttr(4);
            judgingPlayerPotential = genAttr(4);
            break;
        case StaffRole::HeadOfYouthDevelopment:
            workingWithYoungsters = genAttr(6);
            judgingPlayerPotential = genAttr(3);
            break;
    }
}

int Staff::getYouthIntakeModifier() const {
    if (role != StaffRole::HeadOfYouthDevelopment) return 0;
    // A 20-rated HOYD can add up to +15 PA to the upper bound of a youth intake player
    return static_cast<int>((workingWithYoungsters / 20.0f) * 15.0f);
}

float Staff::getPhysioRecoveryModifier() const {
    if (role != StaffRole::Physio) return 0.0f;
    // A 20-rated Physio shaves off 25% of the injury duration
    return (physiotherapy / 20.0f) * 0.25f;
}

nlohmann::json Staff::toJson() const {
    return {
        {"id", uniqueId}, {"name", name}, {"role", static_cast<int>(role)},
        {"att", attackingCoaching}, {"def", defendingCoaching}, {"fit", fitnessCoaching}, {"gk", goalkeepingCoaching},
        {"phys", physiotherapy}, {"jpa", judgingPlayerAbility}, {"jpp", judgingPlayerPotential}, {"wwy", workingWithYoungsters},
        {"wage", wage}
    };
}

void Staff::fromJson(const nlohmann::json& j) {
    uniqueId = j.value("id", "");
    name = j.value("name", "Unknown");
    role = static_cast<StaffRole>(j.value("role", 0));
    attackingCoaching = j.value("att", 1);
    defendingCoaching = j.value("def", 1);
    fitnessCoaching = j.value("fit", 1);
    goalkeepingCoaching = j.value("gk", 1);
    physiotherapy = j.value("phys", 1);
    judgingPlayerAbility = j.value("jpa", 1);
    judgingPlayerPotential = j.value("jpp", 1);
    workingWithYoungsters = j.value("wwy", 1);
    wage = j.value("wage", 0LL);
}
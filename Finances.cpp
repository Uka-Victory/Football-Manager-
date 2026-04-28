#include "Finances.hpp"

ClubFinances::ClubFinances() 
    : balance(0), transferBudget(0), wageBudget(0),
      sponsorshipIncome(0), matchdayIncome(0), prizeMoney(0),
      wageExpenses(0), transferExpenses(0), facilityExpenses(0) {}

int ClubFinances::getTotalWeeklyWages() const {
    int total = 0;
    for (const auto& pair : contracts) {
        total += pair.second.weeklyWage;
    }
    return total;
}

bool ClubFinances::isWithinWageBudget() const {
    return getTotalWeeklyWages() <= wageBudget;
}
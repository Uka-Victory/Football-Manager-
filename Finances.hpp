#ifndef FINANCES_HPP
#define FINANCES_HPP

#include <string>
#include <map>

struct WageContract {
    std::string playerId;
    std::string playerName;
    int weeklyWage;
    std::string contractEnd;
    bool isYouth;
};

class ClubFinances {
public:
    int balance;
    int transferBudget;
    int wageBudget;
    
    int sponsorshipIncome;
    int matchdayIncome;
    int prizeMoney;
    int wageExpenses;
    int transferExpenses;
    int facilityExpenses;
    
    std::map<std::string, WageContract> contracts;
    
    ClubFinances();
    int getTotalWeeklyWages() const;
    bool isWithinWageBudget() const;
};

#endif
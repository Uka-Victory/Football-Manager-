// Finances.hpp
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "json.hpp"
using json = nlohmann::json;

struct SponsorshipDeal {
    std::string name;
    int64_t annualValue = 0;
    int yearsRemaining = 3;
};

class Finances {
private:
    int64_t m_totalBalance = 0;
    int64_t m_wageBudget = 0;
    int64_t m_transferBudget = 0;
    int64_t m_currentWageSpend = 0;
    int64_t m_matchdayIncome = 0;
    int64_t m_broadcastingRevenue = 0;
    int64_t m_commercialRevenue = 0;
    int64_t m_prizeMoney = 0;
    int64_t m_playerSales = 0;
    int64_t m_playerPurchases = 0;
    int64_t m_facilityMaintenance = 0;
    int64_t m_youthAcademyCost = 0;
    int64_t m_staffWages = 0;
    std::vector<SponsorshipDeal> m_sponsorships;
    int64_t m_loanRepayments = 0;
    int m_ffpAccumulatedDeficit = 0;
    bool m_underTransferEmbargo = false;

public:
    Finances();
    void initialiseBudgets(int clubLevel, int reputation, int stadiumCapacity);

    // Revenue
    void processMatchdayIncome(int attendance, int ticketPrice);
    void addBroadcastingRevenue(int64_t amount);
    void addCommercialRevenue(int64_t amount);
    void addPrizeMoney(int64_t amount);
    void addPlayerSale(int64_t fee);
    void addSponsorshipDeal(const std::string& name, int64_t annualValue, int years);
    void renewSponsorships();

    // Expenditure
    void deductPlayerWages(int64_t weeklyWageTotal);
    void deductStaffWages(int64_t monthlyTotal);
    void deductFacilityMaintenance(int64_t monthlyTotal);
    void deductYouthAcademyCost(int64_t annualCost);
    void makeTransferPayment(int64_t fee);
    void makeLoanRepayment(int64_t amount);

    // Budgeting
    void recomputeBudgets();
    void setWageBudget(int64_t wb) { m_wageBudget = wb; }
    void setTransferBudget(int64_t tb) { m_transferBudget = tb; }
    int64_t getTotalBalance() const { return m_totalBalance; }
    int64_t getWageBudget() const { return m_wageBudget; }
    int64_t getTransferBudget() const { return m_transferBudget; }
    int64_t getCurrentWageSpend() const { return m_currentWageSpend; }

    // FFP
    void updateFFP(int64_t seasonProfitLoss);
    bool isUnderTransferEmbargo() const { return m_underTransferEmbargo; }

    // Season lifecycle
    void resetSeasonalRevenue();
    void applySeasonEndFinancials();

    // Serialisation
    json toJson() const;
    void fromJson(const json& j);
};
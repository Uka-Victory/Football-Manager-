// Finances.cpp
#include "Finances.hpp"
#include <algorithm>

Finances::Finances() {}

void Finances::initialiseBudgets(int clubLevel, int reputation, int stadiumCapacity) {
    int64_t baseRevenue = static_cast<int64_t>(clubLevel) * 5'000'000
                        + static_cast<int64_t>(reputation) * 500'000;
    m_totalBalance = baseRevenue * 2;
    m_wageBudget = baseRevenue / 52;
    m_transferBudget = baseRevenue / 2;
}

void Finances::processMatchdayIncome(int attendance, int ticketPrice) {
    int64_t income = static_cast<int64_t>(attendance) * ticketPrice;
    m_matchdayIncome += income;
    m_totalBalance += income;
}

void Finances::addBroadcastingRevenue(int64_t amount) {
    m_broadcastingRevenue += amount;
    m_totalBalance += amount;
}

void Finances::addCommercialRevenue(int64_t amount) {
    m_commercialRevenue += amount;
    m_totalBalance += amount;
}

void Finances::addPrizeMoney(int64_t amount) {
    m_prizeMoney += amount;
    m_totalBalance += amount;
}

void Finances::addPlayerSale(int64_t fee) {
    m_playerSales += fee;
    m_totalBalance += fee;
}

void Finances::addSponsorshipDeal(const std::string& name, int64_t annualValue, int years) {
    SponsorshipDeal deal;
    deal.name = name;
    deal.annualValue = annualValue;
    deal.yearsRemaining = years;
    m_sponsorships.push_back(deal);
    m_totalBalance += annualValue;
    m_commercialRevenue += annualValue;
}

void Finances::renewSponsorships() {
    for (auto& deal : m_sponsorships) {
        if (deal.yearsRemaining > 0) {
            deal.yearsRemaining--;
            m_totalBalance += deal.annualValue;
            m_commercialRevenue += deal.annualValue;
        }
    }
    m_sponsorships.erase(
        std::remove_if(m_sponsorships.begin(), m_sponsorships.end(),
            [](const SponsorshipDeal& d) { return d.yearsRemaining <= 0; }),
        m_sponsorships.end());
}

void Finances::deductPlayerWages(int64_t weeklyWageTotal) {
    m_currentWageSpend = weeklyWageTotal;
    m_totalBalance -= weeklyWageTotal;
}

void Finances::deductStaffWages(int64_t monthlyTotal) {
    m_staffWages += monthlyTotal;
    m_totalBalance -= monthlyTotal;
}

void Finances::deductFacilityMaintenance(int64_t monthlyTotal) {
    m_facilityMaintenance += monthlyTotal;
    m_totalBalance -= monthlyTotal;
}

void Finances::deductYouthAcademyCost(int64_t annualCost) {
    m_youthAcademyCost += annualCost;
    m_totalBalance -= annualCost;
}

void Finances::makeTransferPayment(int64_t fee) {
    m_playerPurchases += fee;
    m_totalBalance -= fee;
}

void Finances::makeLoanRepayment(int64_t amount) {
    m_loanRepayments += amount;
    m_totalBalance -= amount;
}

void Finances::recomputeBudgets() {
    int64_t estimatedRevenue = m_matchdayIncome + m_broadcastingRevenue + m_commercialRevenue;
    m_wageBudget = std::max(int64_t(0), estimatedRevenue / 52);
    m_transferBudget = std::max(int64_t(0), m_totalBalance * 40 / 100);
}

void Finances::updateFFP(int64_t seasonProfitLoss) {
    if (seasonProfitLoss < 0)
        m_ffpAccumulatedDeficit += -seasonProfitLoss;
    else
        m_ffpAccumulatedDeficit = std::max(0, m_ffpAccumulatedDeficit - static_cast<int>(seasonProfitLoss));
    if (m_ffpAccumulatedDeficit > 100'000'000)
        m_underTransferEmbargo = true;
}

void Finances::resetSeasonalRevenue() {
    m_matchdayIncome = 0;
    m_broadcastingRevenue = 0;
    m_commercialRevenue = 0;
    m_prizeMoney = 0;
    m_playerSales = 0;
    m_playerPurchases = 0;
    m_facilityMaintenance = 0;
    m_youthAcademyCost = 0;
    m_staffWages = 0;
}

void Finances::applySeasonEndFinancials() {
    recomputeBudgets();
    resetSeasonalRevenue();
}

json Finances::toJson() const {
    json j;
    j["totalBalance"] = m_totalBalance;
    j["wageBudget"] = m_wageBudget;
    j["transferBudget"] = m_transferBudget;
    j["currentWageSpend"] = m_currentWageSpend;
    j["sponsorships"] = json::array();
    for (const auto& s : m_sponsorships) {
        j["sponsorships"].push_back({
            {"name", s.name},
            {"annualValue", s.annualValue},
            {"yearsRemaining", s.yearsRemaining}
        });
    }
    j["ffpAccumulatedDeficit"] = m_ffpAccumulatedDeficit;
    j["underTransferEmbargo"] = m_underTransferEmbargo;
    return j;
}

void Finances::fromJson(const json& j) {
    m_totalBalance = j.value("totalBalance", 0);
    m_wageBudget = j.value("wageBudget", 0);
    m_transferBudget = j.value("transferBudget", 0);
    m_currentWageSpend = j.value("currentWageSpend", 0);
    m_sponsorships.clear();
    if (j.contains("sponsorships") && j["sponsorships"].is_array()) {
        for (const auto& sj : j["sponsorships"]) {
            SponsorshipDeal deal;
            deal.name = sj.value("name", "");
            deal.annualValue = sj.value("annualValue", 0);
            deal.yearsRemaining = sj.value("yearsRemaining", 0);
            m_sponsorships.push_back(deal);
        }
    }
    m_ffpAccumulatedDeficit = j.value("ffpAccumulatedDeficit", 0);
    m_underTransferEmbargo = j.value("underTransferEmbargo", false);
}
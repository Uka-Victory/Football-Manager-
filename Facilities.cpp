#include "Finances.hpp"
#include <algorithm>

namespace FootballManager {

    Finances::Finances(long long initialBalance, long long initialWageBudget)
        : balance(initialBalance), wageBudget(initialWageBudget), currentWageSpend(0) {}

    void Finances::addWage(long long amount) {
        currentWageSpend += amount;
    }

    void Finances::removeWage(long long amount) {
        currentWageSpend = std::max(0LL, currentWageSpend - amount);
    }

    bool Finances::processTransfer(long long fee) {
        if (balance >= fee) {
            balance -= fee;
            return true;
        }
        return false;
    }

} // namespace FootballManager
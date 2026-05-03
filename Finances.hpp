#pragma once

namespace FootballManager {

    class Finances {
    private:
        long long balance;
        long long wageBudget;
        long long currentWageSpend;

    public:
        Finances(long long initialBalance, long long initialWageBudget);

        long long getBalance() const { return balance; }
        long long getWageBudget() const { return wageBudget; }
        long long getAvailableWageBudget() const { return wageBudget - currentWageSpend; }

        void addWage(long long amount);
        void removeWage(long long amount);
        bool processTransfer(long long fee);
    };

} // namespace FootballManager
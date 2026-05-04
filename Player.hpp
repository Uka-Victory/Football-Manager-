#pragma once
#include <string>
#include <vector>
#include <memory>

namespace FootballManager {

    enum class Position { GK, CB, LB, RB, DM, CM, AM, RW, LW, ST };

    enum class SquadStatus {
        EmergencyProspect = 2,
        Backup = 6,
        SquadPlayer = 11,
        ImportantPlayer = 16,
        StarPlayer = 20
    };

    struct CurrentSeasonStats {
        int appearances = 0;
        int goals = 0;
        int assists = 0;
        int cleanSheets = 0;
        int yellowCards = 0;
        int redCards = 0;
        double averageRating = 0.0;
        double accumulatedXG = 0.0;
        int keyPasses = 0;

        void reset();
        void updateRating(double newMatchRating);
    };

    class Player {
    private:
        std::string id;
        std::string name;
        int age;
        Position primaryPosition;

        // 1-20 Atomic Attributes
        int finishing;
        int passing;
        int tackling;
        int pace;
        int vision;
        int composure;
        int decisions;
        int anticipation;
        int positioning;

        // Hidden Reputations (1-20)
        int currentReputation;
        int homeReputation;
        int worldReputation;

        int weeklyWage;
        int contractEndYear;
        int academyJoinYear;
        SquadStatus status;
        int manualRank;
        int fitness;

        CurrentSeasonStats seasonStats;

    public:
        Player(std::string playerId, std::string playerName, int playerAge, Position pos);

        std::string getId()              const { return id; }
        std::string getName()            const { return name; }
        Position getPrimaryPosition()    const { return primaryPosition; }
        int getFitness()                 const { return fitness; }
        int getManualRank()              const { return manualRank; }
        SquadStatus getStatus()          const { return status; }
        int getAttribute(const std::string& attrName) const;
        int getWeeklyWage()              const { return weeklyWage; }
        int getAcademyJoinYear()         const { return academyJoinYear; }
        CurrentSeasonStats& getStats()         { return seasonStats; }

        void setManualRank(int rank)             { manualRank = rank; }
        void setStatus(SquadStatus newStatus)    { status = newStatus; }
        void setFitness(int newFitness)          { fitness = newFitness; }
        void assignContract(int wage, int endYear);
        void setAcademyJoinYear(int year)        { academyJoinYear = year; }
        void setAttribute(const std::string& attrName, int newValue);

        double calculateRoleScore(Position role) const;
        double getSelectionIndex() const;
        void processMidnightWipe();
    };

    // Fix: Define PlayerPtr here so all files that include Player.hpp can use it
    using PlayerPtr = std::shared_ptr<Player>;

} // namespace FootballManager
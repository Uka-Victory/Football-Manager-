#pragma once
#include "Team.hpp"
#include "WorldData.hpp"
#include <map>
#include <string>
#include <vector>
#include <utility>

namespace FootballManager {

    struct HierarchyReport {
        std::string playerName;
        Position pos;
        SquadStatus status;
        int manualRank;
        double averageRating;
        bool isConflict; // True if a Star Player is manually ranked low
    };

    class DataHub {
    private:
        // 12x8 Grid tracking for failures (x: 1-12, y: 1-8)
        std::map<std::pair<int, int>, int> zoneFailures;
        int totalPasses;
        int forwardGridUnitsProgressed;

    public:
        DataHub();

        // 1. Tactical Forensics
        void logDuelFailure(int x, int y);
        void logPassProgression(int startX, int endX);
        std::vector<std::pair<std::pair<int, int>, int>> getSoftZones() const; // Returns worst grid coordinates
        double getVerticalityIndex() const; // Forward units per pass

        // 2. Performance Analytics
        double calculateClinicalDelta(Team& team) const; // Actual Goals - Expected Goals (xG)
        std::vector<PlayerPtr> getSlumpingPlayers(Team& team) const; // Avg Rating < 6.4

        // 3. Hierarchy Alignment
        std::vector<HierarchyReport> generateHierarchyReport(Team& team) const;

        // 4. Scouting & Market
        std::vector<PlayerPtr> generateWatchlistReport(WorldData& worldData) const;
        
        void resetSeasonalData();
    };

} // namespace FootballManager
#include "DataHub.hpp"
#include <algorithm>

namespace FootballManager {

    DataHub::DataHub() : totalPasses(0), forwardGridUnitsProgressed(0) {}

    void DataHub::logDuelFailure(int x, int y) {
        if (x >= 1 && x <= 12 && y >= 1 && y <= 8) {
            zoneFailures[{x, y}]++;
        }
    }

    void DataHub::logPassProgression(int startX, int endX) {
        totalPasses++;
        if (endX > startX) {
            forwardGridUnitsProgressed += (endX - startX);
        }
    }

    std::vector<std::pair<std::pair<int, int>, int>> DataHub::getSoftZones() const {
        std::vector<std::pair<std::pair<int, int>, int>> sortedZones(zoneFailures.begin(), zoneFailures.end());
        
        // Sort by highest number of failures
        std::sort(sortedZones.begin(), sortedZones.end(), 
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });
            
        // Return top 5 worst zones
        if (sortedZones.size() > 5) {
            sortedZones.resize(5);
        }
        return sortedZones;
    }

    double DataHub::getVerticalityIndex() const {
        if (totalPasses == 0) return 0.0;
        return static_cast<double>(forwardGridUnitsProgressed) / totalPasses;
    }

    double DataHub::calculateClinicalDelta(Team& team) const {
        double totalGoals = 0.0;
        double totalXG = 0.0;
        
        // Extract all players including youth for complete team stats
        std::vector<PlayerPtr> roster = team.getBestXI(); // Using starting XI as core metric
        
        for (const auto& player : roster) {
            totalGoals += player->getStats().goals;
            totalXG += player->getStats().accumulatedXG;
        }
        
        return totalGoals - totalXG;
    }

    std::vector<PlayerPtr> DataHub::getSlumpingPlayers(Team& team) const {
        std::vector<PlayerPtr> slumping;
        std::vector<PlayerPtr> roster = team.getBestXI();
        
        for (const auto& player : roster) {
            if (player->getStats().appearances > 3 && player->getStats().averageRating < 6.4) {
                slumping.push_back(player);
            }
        }
        return slumping;
    }

    std::vector<HierarchyReport> DataHub::generateHierarchyReport(Team& team) const {
        std::vector<HierarchyReport> report;
        std::vector<PlayerPtr> roster = team.getBestXI(); // Pull active starters

        for (const auto& player : roster) {
            HierarchyReport rep;
            rep.playerName = player->getName();
            rep.pos = player->getPrimaryPosition();
            rep.status = player->getStatus();
            rep.manualRank = player->getManualRank();
            rep.averageRating = player->getStats().averageRating;
            
            // Flag conflict: If Star Player is manually ranked 3rd or lower
            rep.isConflict = (rep.status == SquadStatus::StarPlayer && rep.manualRank >= 3);
            report.push_back(rep);
        }
        return report;
    }

    std::vector<PlayerPtr> DataHub::generateWatchlistReport(WorldData& worldData) const {
        std::vector<PlayerPtr> reportPlayers;
        for (const std::string& id : worldData.getWatchlist()) {
            PlayerPtr target = worldData.findPlayerGlobally(id);
            if (target) {
                reportPlayers.push_back(target);
            }
        }
        return reportPlayers;
    }

    void DataHub::resetSeasonalData() {
        zoneFailures.clear();
        totalPasses = 0;
        forwardGridUnitsProgressed = 0;
    }

} // namespace FootballManager
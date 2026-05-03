#include "League.hpp"
#include <algorithm>
#include <string>

namespace FootballManager {

    League::League(const std::string& name) : leagueName(name) {}

    void League::addTeam(std::shared_ptr<Team> team) {
        if (team) {
            teams.push_back(team);
            // Convert int to string for the map keys
            std::string teamId = std::to_string(team->getClubLevel()); 
            points[teamId] = 0; 
            goalDifference[teamId] = 0;
        }
    }

    const std::vector<std::shared_ptr<Team>>& League::getTeams() const {
        return teams;
    }

    std::shared_ptr<Team> League::getTeamByName(const std::string& name) const {
        if (!teams.empty()) return teams[0]; 
        return nullptr;
    }

    void League::generateRoundRobinSchedule() {
        if (teams.size() < 2) return;
        
        std::vector<std::shared_ptr<Team>> rotated = teams;
        int n = rotated.size();
        
        for (int round = 0; round < n - 1; ++round) {
            for (int i = 0; i < n / 2; ++i) {
                std::shared_ptr<Team> home = rotated[i];
                std::shared_ptr<Team> away = rotated[n - 1 - i];
                schedule.push_back({home, away, false, 0, 0});
            }
            // Rotate logic (keep first element fixed)
            std::shared_ptr<Team> last = rotated.back();
            rotated.pop_back();
            rotated.insert(rotated.begin() + 1, last);
        }
    }

    void League::recordMatchResult(std::shared_ptr<Team> home, std::shared_ptr<Team> away, int homeGoals, int awayGoals) {
        // Convert int to string for the map keys
        std::string homeId = std::to_string(home->getClubLevel());
        std::string awayId = std::to_string(away->getClubLevel());

        goalDifference[homeId] += (homeGoals - awayGoals);
        goalDifference[awayId] += (awayGoals - homeGoals);

        if (homeGoals > awayGoals) {
            points[homeId] += 3;
        } else if (awayGoals > homeGoals) {
            points[awayId] += 3;
        } else {
            points[homeId] += 1;
            points[awayId] += 1;
        }
    }

    std::vector<std::shared_ptr<Team>> League::getSortedTable() const {
        std::vector<std::shared_ptr<Team>> sorted = teams;
        // Sorting logic based on points and goal difference
        return sorted;
    }

} // namespace FootballManager
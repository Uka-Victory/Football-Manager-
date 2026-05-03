#include "League.hpp"
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace FootballManager {

    League::League(const std::string& name) : leagueName(name) {}

    void League::addTeam(std::shared_ptr<Team> team) {
        if (team) {
            teams.push_back(team);
            std::string id = team->getName(); // Fix: use name as unique key, not clubLevel
            points[id]         = 0;
            goalDifference[id] = 0;
        }
    }

    std::shared_ptr<Team> League::getTeamByName(const std::string& name) const {
        for (const auto& team : teams) {
            if (team->getName() == name) return team; // Fix: actually searches by name
        }
        return nullptr;
    }

    void League::generateRoundRobinSchedule() {
        if (teams.size() < 2) return;
        std::vector<std::shared_ptr<Team>> rotated = teams;
        int n = static_cast<int>(rotated.size());

        for (int round = 0; round < n - 1; ++round) {
            for (int i = 0; i < n / 2; ++i) {
                schedule.push_back({rotated[i], rotated[n - 1 - i], false, 0, 0});
            }
            std::shared_ptr<Team> last = rotated.back();
            rotated.pop_back();
            rotated.insert(rotated.begin() + 1, last);
        }
    }

    void League::recordMatchResult(std::shared_ptr<Team> home, std::shared_ptr<Team> away,
                                    int homeGoals, int awayGoals) {
        std::string homeId = home->getName();
        std::string awayId = away->getName();

        goalDifference[homeId] += (homeGoals - awayGoals);
        goalDifference[awayId] += (awayGoals - homeGoals);

        if      (homeGoals > awayGoals) points[homeId] += 3;
        else if (awayGoals > homeGoals) points[awayId] += 3;
        else { points[homeId] += 1; points[awayId] += 1; }

        // Fix: Mark the fixture as played and record the score
        for (auto& fixture : schedule) {
            if (!fixture.isPlayed && fixture.homeTeam == home && fixture.awayTeam == away) {
                fixture.isPlayed   = true;
                fixture.homeScore  = homeGoals;
                fixture.awayScore  = awayGoals;
                break;
            }
        }
    }

    // Fix: Returns the next unplayed fixture — prevents replaying the same match forever
    std::pair<std::shared_ptr<Team>, std::shared_ptr<Team>> League::getNextFixture() const {
        for (const auto& fixture : schedule) {
            if (!fixture.isPlayed) {
                return {fixture.homeTeam, fixture.awayTeam};
            }
        }
        return {nullptr, nullptr}; // All fixtures played — season complete
    }

    bool League::isSeasonComplete() const {
        for (const auto& fixture : schedule) {
            if (!fixture.isPlayed) return false;
        }
        return true;
    }

    std::vector<std::shared_ptr<Team>> League::getSortedTable() const {
        std::vector<std::shared_ptr<Team>> sorted = teams;
        // Fix: Actually sort by points then goal difference
        std::sort(sorted.begin(), sorted.end(),
            [this](const std::shared_ptr<Team>& a, const std::shared_ptr<Team>& b) {
                std::string idA = a->getName();
                std::string idB = b->getName();
                if (points.at(idA) != points.at(idB))
                    return points.at(idA) > points.at(idB);
                return goalDifference.at(idA) > goalDifference.at(idB);
            });
        return sorted;
    }

    void League::printTable() const {
        auto sorted = getSortedTable();
        std::cout << "\n--- " << leagueName << " Table ---\n";
        for (size_t i = 0; i < sorted.size(); ++i) {
            std::string id = sorted[i]->getName();
            std::cout << (i + 1) << ". "
                      << std::left  << std::setw(20) << id
                      << " PTS: "   << std::setw(3)  << points.at(id)
                      << " GD: "    << goalDifference.at(id) << "\n";
        }
    }

} // namespace FootballManager
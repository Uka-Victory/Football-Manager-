#pragma once
#include "Team.hpp"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace FootballManager {

    struct Fixture {
        std::shared_ptr<Team> homeTeam;
        std::shared_ptr<Team> awayTeam;
        bool isPlayed;
        int homeScore;
        int awayScore;
    };

    class League {
    private:
        std::string leagueName;
        std::vector<std::shared_ptr<Team>> teams;
        std::vector<Fixture> schedule;
        
        // League Table Tracking
        std::unordered_map<std::string, int> points;
        std::unordered_map<std::string, int> goalDifference;

    public:
        League(const std::string& name);

        void addTeam(std::shared_ptr<Team> team);
        const std::vector<std::shared_ptr<Team>>& getTeams() const;
        std::shared_ptr<Team> getTeamByName(const std::string& name) const;
        
        void generateRoundRobinSchedule();
        void recordMatchResult(std::shared_ptr<Team> home, std::shared_ptr<Team> away, int homeGoals, int awayGoals);
        
        std::vector<std::shared_ptr<Team>> getSortedTable() const;
    };

} // namespace FootballManager
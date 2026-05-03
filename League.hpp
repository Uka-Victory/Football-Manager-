#pragma once
#include "Team.hpp"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <utility>

namespace FootballManager {

    struct Fixture {
        std::shared_ptr<Team> homeTeam;
        std::shared_ptr<Team> awayTeam;
        bool isPlayed;
        int  homeScore;
        int  awayScore;
    };

    class League {
    private:
        std::string leagueName;
        std::vector<std::shared_ptr<Team>> teams;
        std::vector<Fixture> schedule;

        std::unordered_map<std::string, int> points;
        std::unordered_map<std::string, int> goalDifference;

    public:
        League(const std::string& name);

        void addTeam(std::shared_ptr<Team> team);
        std::shared_ptr<Team> getTeamByName(const std::string& name) const;

        void generateRoundRobinSchedule();
        void recordMatchResult(std::shared_ptr<Team> home, std::shared_ptr<Team> away,
                               int homeGoals, int awayGoals);

        // Fix: Returns the next unplayed fixture so the game loop always advances correctly
        std::pair<std::shared_ptr<Team>, std::shared_ptr<Team>> getNextFixture() const;

        std::vector<std::shared_ptr<Team>> getSortedTable() const;
        void printTable() const;
        bool isSeasonComplete() const;
    };

} // namespace FootballManager
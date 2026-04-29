#include "League.hpp"
#include <algorithm>

League::League(std::string leagueName, int tierLevel) 
    : name(leagueName), level(tierLevel) {}

std::string League::getName() const { return name; }
int League::getLevel() const { return level; }

void League::addTeam(TeamPtr team) {
    if (!team) return;
    teams.push_back(team);
    // Initialize their record for the season
    standings[team->getName()] = LeagueRecord();
}

const std::vector<TeamPtr>& League::getTeams() const {
    return teams;
}

TeamPtr League::getTeamByName(const std::string& teamName) const {
    for (const auto& t : teams) {
        if (t->getName() == teamName) {
            return t;
        }
    }
    return nullptr;
}

void League::recordMatch(const std::string& homeTeamName, const std::string& awayTeamName, int homeGoals, int awayGoals) {
    auto& homeRecord = standings[homeTeamName];
    auto& awayRecord = standings[awayTeamName];

    homeRecord.played++;
    awayRecord.played++;
    
    homeRecord.goalsFor += homeGoals;
    homeRecord.goalsAgainst += awayGoals;
    
    awayRecord.goalsFor += awayGoals;
    awayRecord.goalsAgainst += homeGoals;

    if (homeGoals > awayGoals) {
        homeRecord.won++;
        homeRecord.points += 3;
        awayRecord.lost++;
    } else if (awayGoals > homeGoals) {
        awayRecord.won++;
        awayRecord.points += 3;
        homeRecord.lost++;
    } else {
        homeRecord.drawn++;
        homeRecord.points += 1;
        awayRecord.drawn++;
        awayRecord.points += 1;
    }
}

void League::resetSeason() {
    // Keep the teams, but clear their wins/losses/points back to 0
    standings.clear();
    for (const auto& t : teams) {
        standings[t->getName()] = LeagueRecord();
    }
}

std::vector<std::pair<TeamPtr, LeagueRecord>> League::getSortedStandings() const {
    std::vector<std::pair<TeamPtr, LeagueRecord>> sortedTable;
    
    for (const auto& t : teams) {
        sortedTable.push_back({t, standings.at(t->getName())});
    }

    // Sort by Points -> Goal Difference -> Goals Scored
    std::sort(sortedTable.begin(), sortedTable.end(), [](const auto& a, const auto& b) {
        if (a.second.points != b.second.points)
            return a.second.points > b.second.points;
        if (a.second.goalDifference() != b.second.goalDifference())
            return a.second.goalDifference() > b.second.goalDifference();
        return a.second.goalsFor > b.second.goalsFor;
    });

    return sortedTable;
}

// --- SAVE / LOAD SYSTEM ---

nlohmann::json League::toJson() const {
    nlohmann::json j;
    j["name"] = name;
    j["level"] = level;
    
    // We only save the team NAMES here. The actual team data is saved by the World object.
    nlohmann::json teamNamesJson = nlohmann::json::array();
    for (const auto& t : teams) {
        teamNamesJson.push_back(t->getName());
    }
    j["teamNames"] = teamNamesJson;

    // Save current season standings
    nlohmann::json standingsJson;
    for (const auto& [tName, record] : standings) {
        standingsJson[tName] = {
            {"played", record.played},
            {"won", record.won},
            {"drawn", record.drawn},
            {"lost", record.lost},
            {"goalsFor", record.goalsFor},
            {"goalsAgainst", record.goalsAgainst},
            {"points", record.points}
        };
    }
    j["standings"] = standingsJson;

    return j;
}

void League::fromJson(const nlohmann::json& j, const std::unordered_map<std::string, TeamPtr>& globalTeamRegistry) {
    name = j.value("name", "Unknown League");
    level = j.value("level", 1);
    
    teams.clear();
    standings.clear();

    // Re-link the teams using the global registry
    if (j.contains("teamNames")) {
        for (const auto& tNameJson : j["teamNames"]) {
            std::string tName = tNameJson.get<std::string>();
            auto it = globalTeamRegistry.find(tName);
            if (it != globalTeamRegistry.end()) {
                teams.push_back(it->second);
            }
        }
    }

    // Restore standings
    if (j.contains("standings")) {
        for (const auto& item : j["standings"].items()) {
            std::string tName = item.key();
            auto recordJson = item.value();
            
            LeagueRecord record;
            record.played = recordJson.value("played", 0);
            record.won = recordJson.value("won", 0);
            record.drawn = recordJson.value("drawn", 0);
            record.lost = recordJson.value("lost", 0);
            record.goalsFor = recordJson.value("goalsFor", 0);
            record.goalsAgainst = recordJson.value("goalsAgainst", 0);
            record.points = recordJson.value("points", 0);
            
            standings[tName] = record;
        }
    } else {
        resetSeason(); // Failsafe: if no standings exist, just start fresh
    }
}
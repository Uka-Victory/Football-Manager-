#include "TeamGenerator.hpp"
#include "Utils.hpp"
#include <map>

TeamGenerator::TeamGenerator(NamePool& np) : namePool(np) {}

static const std::vector<std::string> ALL_POSITIONS = {
    "GK", "RB", "LB", "CB", "DM", "CM", "AM", "LW", "RW", "ST"
};

static const std::vector<std::pair<std::string, std::string>> SENIOR_POSITION_TEMPLATE = {
    {"GK", ""}, {"GK", ""}, {"GK", ""},
    {"RB", ""}, {"RB", ""}, {"LB", ""}, {"LB", ""},
    {"CB", "CCB"}, {"CB", "LCB"}, {"CB", "RCB"}, {"CB", "CCB"}, {"CB", "LCB"},
    {"DM", "CDM"}, {"DM", "LDM"},
    {"CM", "CM"}, {"CM", "LCM"}, {"CM", "RCM"},
    {"AM", "CAM"}, {"AM", "LAM"}, {"AM", "RAM"},
    {"LW", ""}, {"RW", ""},
    {"ST", ""}, {"ST", ""}
};

static const std::vector<std::pair<std::string, std::string>> YOUTH_POSITION_TEMPLATE = {
    {"GK", ""}, {"GK", ""},
    {"RB", ""}, {"LB", ""},
    {"CB", "CCB"}, {"CB", "LCB"}, {"CB", "RCB"},
    {"DM", "CDM"},
    {"CM", "CM"}, {"CM", "RCM"},
    {"AM", "CAM"}, {"AM", "LAM"},
    {"LW", ""}, {"RW", ""},
    {"ST", ""}, {"ST", ""}
};

std::string TeamGenerator::getRandomPosition() {
    std::uniform_int_distribution<size_t> dist(0, ALL_POSITIONS.size() - 1);
    return ALL_POSITIONS[dist(rng)];
}

int TeamGenerator::getOverallForLevel(int level, bool isYouth) {
    // Map level 1-20 to overall range
    // Level 20: 85-95, Level 1: 35-45
    int low = 35 + (level - 1) * 3;
    int high = 45 + (level - 1) * 3;
    if (isYouth) {
        low -= 8;
        high -= 4;
    }
    low = std::max(30, std::min(95, low));
    high = std::max(35, std::min(99, high));
    return randInt(low, high);
}

int TeamGenerator::getInitialBudget(int level) {
    // Level 20: ~200M, Level 1: ~5M
    return 5'000'000 + (level - 1) * 10'000'000;
}

Player TeamGenerator::createSeniorPlayer(const std::string& country, int teamLevel, const std::string& position) {
    std::string name = namePool.getRandomFullName(country);
    int age = randInt(17, 34);
    int overall = getOverallForLevel(teamLevel, false);
    int potential = overall + randInt(0, 8);
    if (potential > 94) potential = 94;
    return Player(name, age, country, position, overall, potential);
}

Player TeamGenerator::createYouthPlayer(const std::string& country, int teamLevel, const std::string& position) {
    std::string name = namePool.getRandomFullName(country);
    int age = randInt(15, 19);
    int overall = getOverallForLevel(teamLevel, true);
    int potential = overall + randInt(5, 15);
    if (potential > 94) potential = 94;
    return Player(name, age, country, position, overall, potential);
}

Team TeamGenerator::generateTeam(const std::string& teamName, const std::string& country, int teamLevel) {
    int budget = getInitialBudget(teamLevel);
    Team team(teamName, country, teamLevel, budget);
    
    // Senior squad: expand template to ~25 players
    int seniorSize = std::max(22, std::min(28, 22 + (teamLevel - 10) / 2));
    std::vector<std::pair<std::string, std::string>> seniorPositions = SENIOR_POSITION_TEMPLATE;
    while ((int)seniorPositions.size() < seniorSize) {
        seniorPositions.push_back({"CM", "CM"});
    }
    
    for (const auto& [pos, role] : seniorPositions) {
        Player p = createSeniorPlayer(country, teamLevel, pos);
        p.positionRole = role;
        team.addPlayer(p);
    }
    
    // Youth squad: ~16 players
    int youthSize = 16;
    std::vector<std::pair<std::string, std::string>> youthPositions = YOUTH_POSITION_TEMPLATE;
    while ((int)youthPositions.size() < youthSize) {
        youthPositions.push_back({"CM", "CM"});
    }
    
    for (const auto& [pos, role] : youthPositions) {
        Player p = createYouthPlayer(country, teamLevel, pos);
        p.positionRole = role;
        team.addYouthPlayer(p);
    }
    
    return team;
}
#include "Team.hpp"
#include <algorithm>

Team::Team(std::string n, std::string c, int lvl, int initialBalance)
    : name(n), country(c), level(lvl), gamesPlayed(0), wins(0), draws(0), losses(0),
      goalsFor(0), goalsAgainst(0), points(0), youthTeamType("U19") {
    finances.balance = initialBalance;
    finances.transferBudget = initialBalance / 2;
    finances.wageBudget = initialBalance / 10;
}

// ... rest unchanged

void Team::addPlayer(const Player& p) {
    players.push_back(p);
}

void Team::addYouthPlayer(const Player& p) {
    youthPlayers.push_back(p);
}

Player* Team::findPlayer(const std::string& playerName) {
    for (auto& p : players) {
        if (p.name == playerName) return &p;
    }
    for (auto& p : youthPlayers) {
        if (p.name == playerName) return &p;
    }
    return nullptr;
}

void Team::updateRecord(int gf, int ga) {
    gamesPlayed++;
    goalsFor += gf;
    goalsAgainst += ga;
    if (gf > ga) { wins++; points += 3; }
    else if (gf == ga) { draws++; points += 1; }
    else { losses++; }
}

int Team::getGoalDifference() const {
    return goalsFor - goalsAgainst;
}
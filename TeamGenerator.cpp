// TeamGenerator.cpp
#include "TeamGenerator.hpp"
#include "Utils.hpp"

std::shared_ptr<Team> TeamGenerator::generateTeam(const TeamInfo& info, int level) {
    auto team = std::make_shared<Team>(info.name, info.country, level);
    int seniorCount = info.seniorCount > 0 ? info.seniorCount : 22;
    int youthCount = info.youthCount > 0 ? info.youthCount : 8;

    for (int i = 0; i < seniorCount; ++i) {
        auto p = std::make_shared<Player>("P_" + std::to_string(Utils::generateUniquePlayerId()));
        p->setName(namePool.generateName(info.country));
        p->setAge(Utils::randInt(18, 34));
        p->setNationality(info.country);
        p->setPrimaryPosition(Utils::randomPosition());
        p->setHomeGrownNation(info.country);
        p->generateAttributes(level, p->getPrimaryPosition());
        p->generatePlaystyle();
        p->generateTraits();
        p->generateHiddenAttributes();
        p->setContractLengthMonths(12 + Utils::randInt(0, 36));
        team->addToSenior(p);
    }
    for (int i = 0; i < youthCount; ++i) {
        auto p = std::make_shared<Player>("P_" + std::to_string(Utils::generateUniquePlayerId()));
        p->setName(namePool.generateName(info.country));
        p->setAge(Utils::randInt(16, 19));
        p->setNationality(info.country);
        p->setPrimaryPosition(Utils::randomPosition());
        p->setHomeGrownNation(info.country);
        p->generateAttributes(std::max(1, level - 3), p->getPrimaryPosition());
        p->generatePlaystyle();
        p->generateTraits();
        p->generateHiddenAttributes();
        p->setContractLengthMonths(12 + Utils::randInt(0, 24));
        team->addToYouth(p);
    }
    team->rebuildDepthChart();
    return team;
}

std::shared_ptr<Player> TeamGenerator::generateYouthPlayer(const std::string& country, int academyLevel) {
    auto p = std::make_shared<Player>("P_" + std::to_string(Utils::generateUniquePlayerId()));
    p->setName(namePool.generateName(country));
    p->setAge(15);
    p->setNationality(country);
    p->setPrimaryPosition(Utils::randomPosition());
    p->setHomeGrownNation(country);
    int level = std::max(1, academyLevel / 2 + Utils::randInt(-2, 2));
    p->generateAttributes(level, p->getPrimaryPosition());
    p->generatePlaystyle();
    p->generateTraits();
    p->generateHiddenAttributes();
    p->setContractLengthMonths(12);
    return p;
}
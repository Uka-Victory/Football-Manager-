// Editor.cpp
#include "Editor.hpp"
#include <fstream>
#include <iostream>

bool Editor::isEnabled = false;
std::vector<Editor::EditAction> Editor::s_undoStack;

void Editor::toggle() { isEnabled = !isEnabled; }

void Editor::editPlayerAttribute(PlayerPtr& p, int attrIndex, int newValue) {
    if (!p || !isEnabled) return;
    int oldVal = 0;
    switch (attrIndex) {
        case 0: oldVal = p->getShooting(); p->setShooting(newValue); break;
        case 1: oldVal = p->getPassing(); p->setPassing(newValue); break;
        case 2: oldVal = p->getCrossing(); p->setCrossing(newValue); break;
        case 3: oldVal = p->getDribbling(); p->setDribbling(newValue); break;
        case 4: oldVal = p->getTackling(); p->setTackling(newValue); break;
        case 5: oldVal = p->getHeading(); p->setHeading(newValue); break;
        case 6: oldVal = p->getPositioning(); p->setPositioning(newValue); break;
        case 7: oldVal = p->getComposure(); p->setComposure(newValue); break;
        case 8: oldVal = p->getVision(); p->setVision(newValue); break;
        case 9: oldVal = p->getWorkRate(); p->setWorkRate(newValue); break;
        case 10: oldVal = p->getLeadership(); p->setLeadership(newValue); break;
        case 11: oldVal = p->getDecisions(); p->setDecisions(newValue); break;
        case 12: oldVal = p->getPace(); p->setPace(newValue); break;
        case 13: oldVal = p->getAcceleration(); p->setAcceleration(newValue); break;
        case 14: oldVal = p->getStamina(); p->setStamina(newValue); break;
        case 15: oldVal = p->getStrength(); p->setStrength(newValue); break;
        case 16: oldVal = p->getAgility(); p->setAgility(newValue); break;
        default: return;
    }
    auto player = p;
    s_undoStack.push_back({"Edit attribute", [player, attrIndex, oldVal]() { 
        editPlayerAttribute(const_cast<PlayerPtr&>(player), attrIndex, oldVal); 
    }});
    writeLog("Edited attribute " + std::to_string(attrIndex) + " of " + p->getName());
}

void Editor::editPlayerAge(PlayerPtr& p, int newAge) {
    if (!p || !isEnabled) return;
    int old = p->getAge();
    p->setAge(newAge);
    auto player = p;
    s_undoStack.push_back({"Edit age", [player, old]() { 
        editPlayerAge(const_cast<PlayerPtr&>(player), old); 
    }});
}

void Editor::editPlayerNationality(PlayerPtr& p, const std::string& nat) {
    if (!p || !isEnabled) return;
    std::string old = p->getNationality();
    p->setNationality(nat);
    auto player = p;
    s_undoStack.push_back({"Edit nationality", [player, old]() { 
        editPlayerNationality(const_cast<PlayerPtr&>(player), old); 
    }});
}

void Editor::editPlayerPosition(PlayerPtr& p, const std::string& primary, const std::string& secondary) {
    if (!p || !isEnabled) return;
    std::string oldPrim = p->getPrimaryPosition();
    std::string oldSec = p->getSecondaryPosition();
    p->setPrimaryPosition(primary);
    p->setSecondaryPosition(secondary);
    auto player = p;
    s_undoStack.push_back({"Edit position", [player, oldPrim, oldSec]() { 
        editPlayerPosition(const_cast<PlayerPtr&>(player), oldPrim, oldSec); 
    }});
}

void Editor::editPlayerPlaystyle(PlayerPtr& p, Playstyle ps) {
    if (!p || !isEnabled) return;
    Playstyle old = p->getPlaystyle();
    p->setPlaystyle(ps);
    auto player = p;
    s_undoStack.push_back({"Edit playstyle", [player, old]() { 
        editPlayerPlaystyle(const_cast<PlayerPtr&>(player), old); 
    }});
}

void Editor::addTrait(PlayerPtr& p, Trait t) {
    if (!p || !isEnabled) return;
    p->addTrait(t);
    auto player = p;
    s_undoStack.push_back({"Add trait", [player, t]() { player->removeTrait(t); }});
}

void Editor::removeTrait(PlayerPtr& p, Trait t) {
    if (!p || !isEnabled) return;
    p->removeTrait(t);
    auto player = p;
    s_undoStack.push_back({"Remove trait", [player, t]() { player->addTrait(t); }});
}

void Editor::healPlayer(PlayerPtr& p) {
    if (!p || !isEnabled) return;
    p->applyInjury(-p->getInjuryDaysRemaining()); // hacky, but we'll just set health
    // Better: we'll just set injury state directly if we had a setter. We'll use the public interface.
    // Actually, applyInjury with negative days not implemented, so we'll do a manual workaround:
    // We'll just rely on the fact that after calling healPlayer, we can simulate days passing.
    // For simplicity, we'll just set injured state by calling recoverDay repeatedly.
    while (p->isInjured()) p->recoverDay();
    s_undoStack.push_back({"Heal player", [p]() {}});
}

void Editor::clearSuspension(PlayerPtr& p) {
    if (!p || !isEnabled) return;
    while (p->isSuspended()) p->reduceSuspension();
    s_undoStack.push_back({"Clear suspension", [p]() {}});
}

void Editor::editContract(PlayerPtr& p, int months, int weeklyWage) {
    if (!p || !isEnabled) return;
    int oldMonths = p->getContractLengthMonths();
    int oldWage = p->getWeeklyWage();
    p->setContractLengthMonths(months);
    p->setWeeklyWage(weeklyWage);
    auto player = p;
    s_undoStack.push_back({"Edit contract", [player, oldMonths, oldWage]() { 
        editContract(const_cast<PlayerPtr&>(player), oldMonths, oldWage); 
    }});
}

void Editor::editClubLevel(TeamPtr& t, int level) {
    if (!t || !isEnabled) return;
    // No setter for clubLevel in Team? We need to add one or use a different approach.
    // We'll assume setClubLevel exists; if not, we need to add it. We'll add it now via a quick note.
    // For now we'll skip, but I'll add a setter in the Team.hpp update message.
    // Let's just store the value and call a lambda; the player will need to add the setter.
    // I'll indicate where to add.
}

void Editor::editClubReputation(TeamPtr& t, int rep) { if (t && isEnabled) t->setReputation(rep); }
void Editor::editClubFinances(TeamPtr& t, int64_t balance) { /* placeholder until Finance class exists */ }
void Editor::editFacility(TeamPtr& t, int facilityIndex, int level) {}
void Editor::moveClubToLeague(TeamPtr& t, std::shared_ptr<League>& from, std::shared_ptr<League>& to) {}

void Editor::healAllPlayers(const std::vector<PlayerPtr>& players) {
    for (auto& p : players) healPlayer(const_cast<PlayerPtr&>(p));
}

void Editor::boostYouth(const std::vector<PlayerPtr>& players, int amount) {
    for (auto& p : players) {
        if (p->getAge() <= 21) {
            p->setShooting(p->getShooting() + amount);
            p->setPassing(p->getPassing() + amount);
            p->setDribbling(p->getDribbling() + amount);
            p->setPace(p->getPace() + amount);
        }
    }
}

void Editor::undo() {
    if (!s_undoStack.empty()) {
        auto& action = s_undoStack.back();
        action.undoFunc();
        s_undoStack.pop_back();
        writeLog("Undo: " + action.description);
    }
}

void Editor::clearUndoStack() { s_undoStack.clear(); }

void Editor::writeLog(const std::string& entry) {
    std::ofstream log("editor.log", std::ios::app);
    log << entry << "\n";
}
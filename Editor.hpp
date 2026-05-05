// Editor.hpp
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Player.hpp"
#include "Team.hpp"
#include "League.hpp"

class Editor {
public:
    static bool isEnabled;
    static void toggle();

    // Player editing
    static void editPlayerAttribute(PlayerPtr& p, int attrIndex, int newValue);
    static void editPlayerAge(PlayerPtr& p, int newAge);
    static void editPlayerNationality(PlayerPtr& p, const std::string& nat);
    static void editPlayerPosition(PlayerPtr& p, const std::string& primary, const std::string& secondary = "");
    static void editPlayerPlaystyle(PlayerPtr& p, Playstyle ps);
    static void addTrait(PlayerPtr& p, Trait t);
    static void removeTrait(PlayerPtr& p, Trait t);
    static void healPlayer(PlayerPtr& p);
    static void clearSuspension(PlayerPtr& p);
    static void editContract(PlayerPtr& p, int months, int weeklyWage);

    // Club editing
    static void editClubLevel(TeamPtr& t, int level);
    static void editClubReputation(TeamPtr& t, int rep);
    static void editClubFinances(TeamPtr& t, int64_t balance);
    static void editFacility(TeamPtr& t, int facilityIndex, int level);
    static void moveClubToLeague(TeamPtr& t, std::shared_ptr<League>& from, std::shared_ptr<League>& to);

    // Mass operations
    static void healAllPlayers(const std::vector<PlayerPtr>& players);
    static void boostYouth(const std::vector<PlayerPtr>& players, int amount);

    // Undo (simple last-action stack)
    static void undo();
    static void clearUndoStack();

    // Logging
    static void writeLog(const std::string& entry);

private:
    struct EditAction {
        std::string description;
        std::function<void()> undoFunc;
    };
    static std::vector<EditAction> s_undoStack;
};
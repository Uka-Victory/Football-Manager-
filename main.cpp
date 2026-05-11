// main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include "Utils.hpp"
#include "WorldData.hpp"
#include "NamePool.hpp"
#include "TeamGenerator.hpp"
#include "League.hpp"
#include "GameCalendar.hpp"
#include "TransferEngine.hpp"
#include "TrainingEngine.hpp"
#include "ProgressionEngine.hpp"
#include "ScoutingEngine.hpp"
#include "MatchEngine.hpp"
#include "Manager.hpp"
#include "WorldHistory.hpp"
#include "DataHub.hpp"
#include "ContinentalManager.hpp"
#include "InternationalManager.hpp"
#include "Finances.hpp"
#include "Tactics.hpp"
#include "Staff.hpp"
#include "Editor.hpp"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

// ========== GLOBAL STATE ==========
vector<LeaguePtr> allLeagues;
map<string, TeamPtr> allTeams;
map<string, PlayerPtr> allPlayers;
WorldData worldData;
NamePool namePool;
TeamGenerator teamGenerator(namePool);
ManagerPtr userManager;
WorldHistoryPtr worldHistory = make_shared<WorldHistory>();
DataHub dataHub;
TeamPtr myClub;
GameCalendar calendar(2025, 7, 1);
ContinentalManager continentalManager(Continent::Europe);
InternationalManager internationalManager;

// User-managed systems
Finances        userFinances;
ScoutingEngine  userScouting;
vector<StaffPtr> userStaff;
TeamTactics     userTactics;
vector<TransferBid> activeBids;

const string SAVE_FILE = "save_game.json";

// ========== HELPERS ==========
static string playstyleStr(Playstyle ps) {
    switch (ps) {
        case Playstyle::DeepLyingPlaymaker:   return "Deep-Lying Playmaker";
        case Playstyle::BoxToBox:             return "Box-to-Box";
        case Playstyle::AnchorMan:            return "Anchor Man";
        case Playstyle::TargetMan:            return "Target Man";
        case Playstyle::Poacher:              return "Poacher";
        case Playstyle::False9:               return "False 9";
        case Playstyle::InsideForward:        return "Inside Forward";
        case Playstyle::Raumdeuter:           return "Raumdeuter";
        case Playstyle::WideWinger:           return "Wide Winger";
        case Playstyle::AdvancedPlaymaker:    return "Advanced Playmaker";
        case Playstyle::DeepLyingForward:     return "Deep-Lying Forward";
        case Playstyle::DefensiveFullBack:    return "Defensive Full-Back";
        case Playstyle::AttackingFullBack:    return "Attacking Full-Back";
        case Playstyle::InvertedWingBack:     return "Inverted Wing-Back";
        case Playstyle::BallPlayingDefender:  return "Ball-Playing Defender";
        case Playstyle::Sweeper:              return "Sweeper";
        case Playstyle::Stopper:              return "Stopper";
        case Playstyle::SweeperKeeper:        return "Sweeper Keeper";
        case Playstyle::TraditionalGoalkeeper:return "Traditional GK";
        default:                              return "Unknown";
    }
}

static string traitStr(Trait t) {
    switch (t) {
        case Trait::FinesseShot:         return "Finesse Shot";
        case Trait::PowerShot:           return "Power Shot";
        case Trait::LongShotTaker:       return "Long Shot Taker";
        case Trait::CurlsBall:           return "Curls Ball";
        case Trait::DivesIntoTackles:    return "Dives Into Tackles";
        case Trait::StaysOnFeet:         return "Stays On Feet";
        case Trait::EarlyCrosser:        return "Early Crosser";
        case Trait::TriesKillerBalls:    return "Tries Killer Balls";
        case Trait::PlaysShortSimplePasses: return "Short Passes";
        case Trait::GiantThrowIn:        return "Giant Throw-In";
        case Trait::InjuryProne:         return "Injury Prone";
        case Trait::Flair:               return "Flair";
        case Trait::TeamPlayer:          return "Team Player";
        case Trait::Selfish:             return "Selfish";
        case Trait::Consistent:          return "Consistent";
        case Trait::Inconsistent:        return "Inconsistent";
        case Trait::PenaltySpecialist:   return "Penalty Specialist";
        case Trait::SetPieceSpecialist:  return "Set Piece Specialist";
        case Trait::OneClubMan:          return "One Club Man";
        case Trait::BigMatchPlayer:      return "Big Match Player";
        default:                         return "Unknown";
    }
}

static void printSeparator(char c = '-', int width = 60) {
    cout << string(width, c) << "\n";
}

static void printHeader(const string& title) {
    printSeparator('=');
    cout << "  " << title << "\n";
    printSeparator('=');
}

// ========== FORWARD DECLARATIONS ==========
void generateWorld();
void selectClub();
void advanceDay();
void saveGame();
void loadGame();
void showMainMenu();
void showMyClubMenu();
void showTransferMenu();
void showScoutingMenu();
void showWorldMenu();
void showInternationalMenu();
void showEditorMenu();

// ========== MAIN ==========
int main() {
    Utils::initRNG();
    printHeader("FOOTBALL MANAGER C++");
    cout << "1. New Game\n2. Load Game\nChoice: ";
    int c; cin >> c;
    userManager = make_shared<Manager>("Player", 40, 50);
    if (c == 2) {
        loadGame();
        if (!myClub) {
            cout << "Load failed - starting new game.\n";
            generateWorld();
            selectClub();
        }
    } else {
        generateWorld();
        selectClub();
        // Initialise user finances based on club level
        if (myClub)
            userFinances.initialiseBudgets(myClub->getClubLevel(),
                                           myClub->getReputation(), 30000);
        userScouting.setClubName(myClub ? myClub->getName() : "");
        userTactics.setFormation("4-3-3");
        saveGame();
    }
    bool running = true;
    while (running) {
        showMainMenu();
        int choice; cin >> choice;
        switch (choice) {
            case 1: advanceDay();           break;
            case 2: showMyClubMenu();       break;
            case 3: showTransferMenu();     break;
            case 4: showScoutingMenu();     break;
            case 5: showWorldMenu();        break;
            case 6: showInternationalMenu();break;
            case 7: showEditorMenu();       break;
            case 8: saveGame();             break;
            case 9: running = false;        break;
            default: cout << "Invalid choice.\n"; break;
        }
    }
    cout << "Thanks for playing!\n";
    return 0;
}

// ========== WORLD GENERATION ==========
void generateWorld() {
    worldData.load("world_data.json");
    namePool.load("countries_data.txt");

    vector<string> allCountryNames;
    const auto& countries = worldData.getBaseCountries();
    for (const auto& country : countries) {
        allCountryNames.push_back(country.name);
        for (const auto& leagueInfo : country.leagues) {
            auto league = make_shared<League>(leagueInfo.name, country.name, leagueInfo.tier);
            league->setPromotionSpots(leagueInfo.promotionSpots);
            league->setRelegationSpots(leagueInfo.relegationSpots);
            for (const auto& teamInfo : leagueInfo.teams) {
                TeamPtr team = teamGenerator.generateTeam(teamInfo, leagueInfo.level);
                team->setPrimaryColour(teamInfo.primaryColour);
                team->setSecondaryColour(teamInfo.secondaryColour);
                team->setStadiumName(teamInfo.stadium);
                team->setFoundedYear(teamInfo.founded);
                league->addTeam(team);
                allTeams[team->getName()] = team;
                for (auto& p : team->getSeniorSquad()) allPlayers[p->getUniqueId()] = p;
                for (auto& p : team->getYouthSquad())   allPlayers[p->getUniqueId()] = p;
            }
            league->generateSchedule(calendar.getYear(), 8, 1);
            allLeagues.push_back(league);
        }
    }
    cout << "Generated " << allLeagues.size() << " leagues with "
         << allTeams.size() << " clubs and " << allPlayers.size() << " players.\n";
    internationalManager.generateNationalTeams(allPlayers, allCountryNames);
}

// ========== CLUB SELECTION ==========
void selectClub() {
    const auto& countries = worldData.getBaseCountries();
    cout << "\nChoose a country:\n";
    for (size_t i = 0; i < countries.size(); ++i)
        cout << i + 1 << ". " << countries[i].name << "\n";
    int ci; cin >> ci;
    if (ci < 1 || ci > (int)countries.size()) ci = 1;
    const auto& country = countries[ci - 1];

    cout << "\nChoose a league:\n";
    for (size_t i = 0; i < country.leagues.size(); ++i)
        cout << i + 1 << ". " << country.leagues[i].name << "\n";
    int li; cin >> li;
    if (li < 1 || li > (int)country.leagues.size()) li = 1;

    LeaguePtr chosenLeague;
    for (auto& league : allLeagues)
        if (league->getName() == country.leagues[li - 1].name &&
            league->getCountry() == country.name) { chosenLeague = league; break; }
    if (!chosenLeague) { cerr << "League not found.\n"; return; }

    cout << "\nChoose a club:\n";
    const auto& teams = chosenLeague->getTeams();
    for (size_t i = 0; i < teams.size(); ++i)
        cout << i + 1 << ". " << teams[i]->getName()
             << " (OVR avg " << [&](){
                 int s=0; for (auto&p:teams[i]->getSeniorSquad()) s+=p->getOverall();
                 return teams[i]->getSeniorSquad().empty() ? 0 :
                        s/(int)teams[i]->getSeniorSquad().size(); }() << ")\n";
    int ti; cin >> ti;
    if (ti < 1 || ti > (int)teams.size()) ti = 1;
    myClub = teams[ti - 1];
    userManager->setCurrentClubName(myClub->getName());
    cout << "\nYou are now managing: " << myClub->getName() << "!\n";
    cout << "League: " << chosenLeague->getName() << " | Country: " << country.name << "\n";
}

// ========== ADVANCE DAY ==========
void advanceDay() {
    string today = calendar.getDateString();
    bool isFirstOfMonth = (calendar.getDay() == 1);
    bool isMonday = (calendar.getWeekday() == 1);

    // --- 1. Domestic fixtures ---
    for (auto& league : allLeagues) {
        auto fixtures = league->getFixturesForDate(today);
        for (auto& fix : fixtures) {
            if (fix.played) continue;
            auto res = MatchEngine::simulateMatch(fix.homeTeam, fix.awayTeam);
            fix.homeGoals = res.homeGoals;
            fix.awayGoals = res.awayGoals;
            fix.played = true;
            league->recordMatchResult(today, fix);
            MatchEngine::applyResultToTeams(fix.homeTeam, fix.awayTeam, res, today);
            fix.homeTeam->updateHeadToHead(fix.awayTeam->getName(),
                fix.homeGoals, fix.awayGoals, fix.homeTeam->getSeniorSquad(), res.playerStats);
            fix.awayTeam->updateHeadToHead(fix.homeTeam->getName(),
                fix.awayGoals, fix.homeGoals, fix.awayTeam->getSeniorSquad(), res.playerStats);
            dataHub.ingestMatch(fix, res);

            bool myMatch = (fix.homeTeam == myClub || fix.awayTeam == myClub);
            if (myMatch) {
                cout << "  [MY CLUB] ";
                // Track manager record
                bool homeWin = (res.homeGoals > res.awayGoals);
                bool draw    = (res.homeGoals == res.awayGoals);
                bool win = (fix.homeTeam == myClub) ? homeWin : !homeWin && !draw;
                userManager->recordResult(win, draw);
            }
            cout << fix.homeTeam->getName() << " " << fix.homeGoals
                 << " - " << fix.awayGoals << " " << fix.awayTeam->getName() << "\n";
        }
    }

    // --- 2. Continental fixtures ---
    for (auto& comp : {"UCL", "UEL", "UECL"}) {
        const auto& contFixtures = continentalManager.getLeaguePhaseFixtures(comp);
        for (auto& fix : contFixtures) {
            if (fix.date == today && !fix.played) {
                auto res = MatchEngine::simulateMatch(fix.homeTeam, fix.awayTeam);
                const_cast<Fixture&>(fix).homeGoals = res.homeGoals;
                const_cast<Fixture&>(fix).awayGoals = res.awayGoals;
                const_cast<Fixture&>(fix).played = true;
                cout << "[" << comp << "] " << fix.homeTeam->getName()
                     << " " << res.homeGoals << " - " << res.awayGoals
                     << " " << fix.awayTeam->getName() << "\n";
            }
        }
    }

    // --- 3. International fixtures ---
    for (auto tournament : {InternationalTournament::WorldCup, InternationalTournament::UEFAEuros,
                            InternationalTournament::CopaAmerica, InternationalTournament::AFCON}) {
        for (const auto& qf : internationalManager.getQualifyingFixtures(tournament)) {
            if (qf.date == today && !qf.played) {
                int hg = Utils::randInt(0, 3), ag = Utils::randInt(0, 3);
                internationalManager.recordTournamentResult(tournament,
                    qf.homeCountry, qf.awayCountry, hg, ag, today);
                cout << "[QUAL] " << qf.homeCountry << " " << hg << "-" << ag << " " << qf.awayCountry << "\n";
            }
        }
        for (const auto& tf : internationalManager.getTournamentFixtures(tournament)) {
            if (tf.date == today && !tf.played) {
                int hg = Utils::randInt(0, 3), ag = Utils::randInt(0, 3);
                internationalManager.recordTournamentResult(tournament,
                    tf.homeCountry, tf.awayCountry, hg, ag, today);
                cout << "[INTL] " << tf.homeCountry << " " << hg << "-" << ag << " " << tf.awayCountry << "\n";
            }
        }
    }

    // --- 4. Training & recovery ---
    for (auto& league : allLeagues)
        for (auto& team : league->getTeams())
            TrainingEngine::processDailyTraining(team);
    for (auto& kv : allPlayers) {
        kv.second->recoverDay();
        kv.second->reduceSuspension();
        kv.second->reduceContract();
    }

    // --- 5. Scouting knowledge gain (weekly) ---
    if (isMonday) userScouting.applyPassiveKnowledgeGain();

    // --- 6. Weekly finances ---
    if (calendar.getWeekday() == 0) {
        int64_t weeklyWages = 0;
        if (myClub)
            for (auto& p : myClub->getSeniorSquad())
                weeklyWages += p->getWeeklyWage();
        userFinances.deductPlayerWages(weeklyWages);
    }

    // --- 7. Monthly systems ---
    if (isFirstOfMonth) {
        int month = calendar.getMonth();
        int year  = calendar.getYear();

        // Progression tick for all players
        vector<PlayerPtr> allPVec;
        allPVec.reserve(allPlayers.size());
        for (auto& kv : allPlayers) allPVec.push_back(kv.second);
        vector<TeamPtr> allTVec;
        allTVec.reserve(allTeams.size());
        for (auto& kv : allTeams) allTVec.push_back(kv.second);
        ProgressionEngine::processMonthlyTick(allPVec, allTVec, month, year);

        // AI transfers (if window is open)
        if (TransferEngine::isTransferWindowOpen(month)) {
            vector<TeamPtr> teamVec(allTVec.begin(), allTVec.end());
            TransferEngine::processAITransfers(teamVec, activeBids, year, month);
        }

        // Staff wages
        int64_t staffCost = 0;
        for (auto& s : userStaff) staffCost += s->getMonthlyWage();
        if (staffCost > 0) userFinances.deductStaffWages(staffCost);

        // Renew sponsorships (January)
        if (month == 1) userFinances.renewSponsorships();
    }

    // --- 8. End of season ---
    if (calendar.isSeasonEnd()) {
        cout << "\n===== END OF SEASON " << calendar.getYear() << " =====\n";

        for (size_t i = 0; i < allLeagues.size(); ++i) {
            League* lower = (i + 1 < allLeagues.size()) ? allLeagues[i + 1].get() : nullptr;
            allLeagues[i]->endSeason(lower);
        }
        worldHistory->computeSeasonAwards(allLeagues, allPlayers);
        worldHistory->saveToFile("history.json");
        continentalManager.updateCountryCoefficients(allLeagues, allTeams);
        continentalManager.updateClubCoefficients(allTeams);

        vector<string> uclQ  = continentalManager.getQualifiedClubs("UCL",  allLeagues, calendar.getYear());
        vector<string> uelQ  = continentalManager.getQualifiedClubs("UEL",  allLeagues, calendar.getYear());
        vector<string> ueclQ = continentalManager.getQualifiedClubs("UECL", allLeagues, calendar.getYear());
        continentalManager.setupChampionsLeague(uclQ,  allTeams, calendar);
        continentalManager.setupEuropaLeague(uelQ,  allTeams, calendar);
        continentalManager.setupConferenceLeague(ueclQ, allTeams, calendar);

        for (auto& league : allLeagues) {
            for (auto& team : league->getTeams()) {
                int intakeCount = 2 + (team->getFacilities().academy / 4);
                for (int j = 0; j < intakeCount; ++j) {
                    auto p = teamGenerator.generateYouthPlayer(team->getCountry(),
                                                               team->getFacilities().academy);
                    team->addToAcademy(p);
                    allPlayers[p->getUniqueId()] = p;
                }
                vector<PlayerPtr> graduates = team->getAcademy();
                vector<string> offered;
                for (size_t k = 0; k < min((size_t)2, graduates.size()); ++k)
                    offered.push_back(graduates[k]->getUniqueId());
                vector<PlayerPtr> released;
                team->processAcademyGraduation(offered, released);
            }
        }

        for (auto& league : allLeagues)
            league->generateSchedule(calendar.getYear() + 1, 8, 1);
        dataHub.resetSeason();

        // Apply season-end finances
        userFinances.applySeasonEndFinancials();
        userFinances.resetSeasonalRevenue();

        // International qualifiers
        int nextYear = calendar.getYear() + 1;
        internationalManager.setYear(nextYear);
        if (internationalManager.isTournamentYear(InternationalTournament::WorldCup))
            internationalManager.setupWorldCupQualifiers(calendar);
        if (internationalManager.isTournamentYear(InternationalTournament::UEFAEuros))
            internationalManager.setupEurosQualifiers(calendar);

        cout << "Season complete! Check World menu for awards.\n";
    }

    calendar.advanceOneDay();
    cout << "Date advanced to " << calendar.getDateString() << "\n";
}

// ========== SAVE / LOAD ==========
void saveGame() {
    json j;
    j["calendar"]             = calendar.toJson();
    j["userManager"]          = userManager->toJson();
    j["myClubName"]           = myClub ? myClub->getName() : "";
    j["userFinances"]         = userFinances.toJson();
    j["userScouting"]         = userScouting.toJson();
    j["leagues"] = json::array();
    for (auto& l : allLeagues) j["leagues"].push_back(l->toJson());
    j["teams"] = json::array();
    for (auto& kv : allTeams) j["teams"].push_back(kv.second->toJson());
    j["players"] = json::array();
    for (auto& kv : allPlayers) {
        json pj;
        nlohmann::adl_serializer<Player>::to_json(pj, *kv.second);
        j["players"].push_back(pj);
    }
    j["worldHistory"]         = worldHistory->toJson();
    j["continentalManager"]   = continentalManager.toJson();
    j["internationalManager"] = internationalManager.toJson();
    j["userTactics"]          = userTactics.toJson();
    j["userStaff"] = json::array();
    for (auto& s : userStaff) j["userStaff"].push_back(s->toJson());

    ofstream out(SAVE_FILE);
    out << j.dump(2);
    cout << "Game saved to " << SAVE_FILE << "\n";
}

void loadGame() {
    ifstream in(SAVE_FILE);
    if (!in) { cout << "No save file found.\n"; return; }
    json j; in >> j;

    calendar.fromJson(j["calendar"]);
    userManager = make_shared<Manager>();
    userManager->fromJson(j["userManager"]);

    allPlayers.clear();
    if (j.contains("players"))
        for (auto& pj : j["players"]) {
            auto p = make_shared<Player>();
            nlohmann::adl_serializer<Player>::from_json(pj, *p);
            allPlayers[p->getUniqueId()] = p;
        }

    allTeams.clear();
    if (j.contains("teams"))
        for (auto& tj : j["teams"]) {
            auto team = Team::fromJson(tj);
            for (auto& p : team->getSeniorSquad()) {
                auto it = allPlayers.find(p->getUniqueId());
                if (it != allPlayers.end()) p = it->second;
            }
            for (auto& p : team->getYouthSquad()) {
                auto it = allPlayers.find(p->getUniqueId());
                if (it != allPlayers.end()) p = it->second;
            }
            for (auto& p : team->getAcademy()) {
                auto it = allPlayers.find(p->getUniqueId());
                if (it != allPlayers.end()) p = it->second;
            }
            allTeams[team->getName()] = team;
        }

    allLeagues.clear();
    if (j.contains("leagues"))
        for (auto& lj : j["leagues"])
            allLeagues.push_back(League::fromJson(lj, allTeams));

    string myClubName = j.value("myClubName", "");
    myClub = allTeams.count(myClubName) ? allTeams[myClubName] : nullptr;
    if (!myClub && !allTeams.empty()) myClub = allTeams.begin()->second;

    if (j.contains("userFinances")) userFinances.fromJson(j["userFinances"]);
    if (j.contains("userScouting")) userScouting.fromJson(j["userScouting"]);
    if (j.contains("userTactics"))  userTactics.fromJson(j["userTactics"]);
    if (j.contains("userStaff"))
        for (auto& sj : j["userStaff"]) {
            auto s = make_shared<Staff>();
            s->fromJson(sj);
            userStaff.push_back(s);
        }

    worldHistory->fromJson(j.value("worldHistory", json::object()));
    continentalManager.fromJson(j.value("continentalManager", json::object()));
    internationalManager.fromJson(j.value("internationalManager", json::object()));
    cout << "Game loaded. Date: " << calendar.getDateString() << "\n";
}

// ========== MAIN MENU ==========
void showMainMenu() {
    cout << "\n";
    printSeparator('=');
    cout << "  Date: " << calendar.getDateString();
    if (myClub) cout << " | Manager: " << userManager->getName()
                     << " | Club: " << myClub->getName();
    cout << "\n";
    if (TransferEngine::isTransferWindowOpen(calendar.getMonth()))
        cout << "  [TRANSFER WINDOW OPEN]\n";
    printSeparator('=');
    cout << "  1. Advance Day\n";
    cout << "  2. My Club\n";
    cout << "  3. Transfers\n";
    cout << "  4. Scouting\n";
    cout << "  5. World\n";
    cout << "  6. International\n";
    cout << "  7. In-Game Editor\n";
    cout << "  8. Save\n";
    cout << "  9. Quit\n";
    printSeparator();
    cout << "Choice: ";
}

// ========== MY CLUB MENU ==========
void showMyClubMenu() {
    if (!myClub) { cout << "No club selected.\n"; return; }
    bool back = false;
    while (!back) {
        printHeader("MY CLUB: " + myClub->getName());
        cout << "  1. View Squad\n";
        cout << "  2. Player Details\n";
        cout << "  3. View Fixtures\n";
        cout << "  4. League Table\n";
        cout << "  5. Tactics\n";
        cout << "  6. Finances\n";
        cout << "  7. Staff\n";
        cout << "  8. Club Records\n";
        cout << "  9. Head-to-Head\n";
        cout << "  0. Back\n";
        cout << "Choice: ";
        int c; cin >> c;
        switch (c) {
            case 0: back = true; break;
            case 1: {
                // Squad list
                printHeader("SQUAD: " + myClub->getName());
                cout << left << setw(4) << "#"
                     << setw(22) << "Name"
                     << setw(5) << "POS"
                     << setw(5) << "OVR"
                     << setw(5) << "AGE"
                     << setw(5) << "FIT"
                     << setw(8) << "Contract"
                     << "Status\n";
                printSeparator();
                const auto& squad = myClub->getSeniorSquad();
                for (size_t i = 0; i < squad.size(); ++i) {
                    const auto& p = squad[i];
                    string status;
                    if (p->isInjured())   status = "INJ(" + to_string(p->getInjuryDaysRemaining()) + "d)";
                    else if (p->isSuspended()) status = "SUSP";
                    else status = "OK";
                    cout << left << setw(4) << (i+1)
                         << setw(22) << p->getName().substr(0,21)
                         << setw(5) << p->getPrimaryPosition()
                         << setw(5) << p->getOverall()
                         << setw(5) << p->getAge()
                         << setw(5) << p->getFitness()
                         << setw(8) << to_string(p->getContractLengthMonths()) + "m"
                         << status << "\n";
                }
                break;
            }
            case 2: {
                // Player details
                cout << "Player number (from squad): ";
                int idx; cin >> idx;
                const auto& squad = myClub->getSeniorSquad();
                if (idx < 1 || idx > (int)squad.size()) { cout << "Invalid.\n"; break; }
                const auto& p = squad[idx - 1];
                printHeader("PLAYER: " + p->getName());
                cout << "  Position:  " << p->getPrimaryPosition();
                if (!p->getSecondaryPosition().empty())
                    cout << " / " << p->getSecondaryPosition();
                cout << "\n";
                cout << "  Age:       " << p->getAge() << "\n";
                cout << "  Nat:       " << p->getNationality() << "\n";
                cout << "  Overall:   " << p->getOverall() << "\n";
                cout << "  Potential: " << p->getPotentialCeiling() << "\n";
                cout << "  Playstyle: " << playstyleStr(p->getPlaystyle()) << "\n";
                cout << "  Fitness:   " << p->getFitness() << "  Morale: " << p->getMorale() << "\n";
                cout << "  Contract:  " << p->getContractLengthMonths() << " months | Wage: "
                     << p->getWeeklyWage() << "/wk\n";
                printSeparator();
                cout << "  ATTRIBUTES\n";
                cout << "  Shooting:" << setw(3) << p->getShooting()
                     << "  Passing:"  << setw(3) << p->getPassing()
                     << "  Dribbling:"<< setw(3) << p->getDribbling() << "\n";
                cout << "  Crossing:" << setw(3) << p->getCrossing()
                     << "  Tackling:" << setw(3) << p->getTackling()
                     << "  Heading:"  << setw(3) << p->getHeading()  << "\n";
                cout << "  Positioning:" << setw(3) << p->getPositioning()
                     << "  Composure:" << setw(3) << p->getComposure()
                     << "  Vision:"    << setw(3) << p->getVision()   << "\n";
                cout << "  Pace:"   << setw(3) << p->getPace()
                     << "  Accel:"  << setw(3) << p->getAcceleration()
                     << "  Stamina:"<< setw(3) << p->getStamina()     << "\n";
                cout << "  Strength:"<< setw(3) << p->getStrength()
                     << "  Agility:" << setw(3) << p->getAgility()
                     << "  WorkRate:"<< setw(3) << p->getWorkRate()   << "\n";
                cout << "  Leadership:"<< setw(3) << p->getLeadership()
                     << "  Decisions:"<< setw(3) << p->getDecisions() << "\n";
                printSeparator();
                cout << "  SEASON STATS\n";
                cout << "  Apps:" << p->getAppearances()
                     << " Goals:" << p->getGoals()
                     << " Assists:" << p->getAssists()
                     << " AvgRating:" << fixed << setprecision(2) << p->getAvgRating() << "\n";
                if (!p->getTraits().empty()) {
                    cout << "  Traits:";
                    for (auto& t : p->getTraits()) cout << " [" << traitStr(t) << "]";
                    cout << "\n";
                }
                break;
            }
            case 3: {
                // Fixtures
                printHeader("FIXTURES: " + myClub->getName());
                for (auto& l : allLeagues) {
                    const auto& teams = l->getTeams();
                    bool inLeague = find(teams.begin(), teams.end(), myClub) != teams.end();
                    if (!inLeague) continue;
                    for (auto& kv : l->getFixtures())
                        for (auto& f : kv.second)
                            if (f.homeTeam == myClub || f.awayTeam == myClub) {
                                cout << kv.first << "  " << f.homeTeam->getName()
                                     << " vs " << f.awayTeam->getName();
                                if (f.played)
                                    cout << " [" << f.homeGoals << "-" << f.awayGoals << "]";
                                cout << "\n";
                            }
                }
                break;
            }
            case 4: {
                // League table
                for (auto& l : allLeagues) {
                    const auto& teams = l->getTeams();
                    bool inLeague = find(teams.begin(), teams.end(), myClub) != teams.end();
                    if (inLeague) { l->printTable(); break; }
                }
                break;
            }
            case 5: {
                // Tactics
                printHeader("TACTICS");
                const auto& form = userTactics.getFormation();
                cout << "  Formation: " << form.name << "\n";
                const auto& instr = userTactics.getInstructions();
                auto mentalityStr = [](Mentality m) -> string {
                    switch(m) {
                        case Mentality::VeryDefensive: return "Very Defensive";
                        case Mentality::Defensive:     return "Defensive";
                        case Mentality::Balanced:      return "Balanced";
                        case Mentality::Attacking:     return "Attacking";
                        case Mentality::VeryAttacking: return "Very Attacking";
                        default: return "Unknown";
                    }
                };
                cout << "  Mentality: " << mentalityStr(instr.mentality) << "\n";
                cout << "  Pressing:  " << instr.pressingIntensity << "/100\n";
                cout << "  Tempo:     " << (instr.tempo==Tempo::Slow?"Slow":instr.tempo==Tempo::Normal?"Normal":"Fast") << "\n";
                printSeparator();
                cout << "  1. Change Formation\n";
                cout << "  2. Change Mentality\n";
                cout << "  3. Change Pressing Intensity\n";
                cout << "  0. Back\n";
                cout << "Choice: ";
                int tc; cin >> tc;
                if (tc == 1) {
                    auto formations = TeamTactics::getPredefinedFormations();
                    for (size_t i = 0; i < formations.size(); ++i)
                        cout << i+1 << ". " << formations[i].name << "\n";
                    cout << "Select: ";
                    int fi; cin >> fi;
                    if (fi >= 1 && fi <= (int)formations.size())
                        userTactics.setFormation(formations[fi-1].name);
                } else if (tc == 2) {
                    cout << "1.Very Def  2.Def  3.Balanced  4.Attack  5.Very Attack\n";
                    cout << "Select: ";
                    int mi; cin >> mi;
                    TeamInstructions ni = instr;
                    switch(mi) {
                        case 1: ni.mentality = Mentality::VeryDefensive;  break;
                        case 2: ni.mentality = Mentality::Defensive;      break;
                        case 3: ni.mentality = Mentality::Balanced;       break;
                        case 4: ni.mentality = Mentality::Attacking;      break;
                        case 5: ni.mentality = Mentality::VeryAttacking;  break;
                        default: break;
                    }
                    userTactics.setInstructions(ni);
                } else if (tc == 3) {
                    cout << "Pressing intensity (0-100): ";
                    int pi; cin >> pi;
                    TeamInstructions ni = instr;
                    ni.pressingIntensity = max(0, min(100, pi));
                    userTactics.setInstructions(ni);
                }
                break;
            }
            case 6: {
                // Finances
                printHeader("FINANCES");
                cout << "  Balance:          " << userFinances.getTotalBalance()     << "\n";
                cout << "  Transfer Budget:  " << userFinances.getTransferBudget()   << "\n";
                cout << "  Wage Budget:      " << userFinances.getWageBudget()       << "\n";
                cout << "  Current Wages:    " << userFinances.getCurrentWageSpend() << "\n";
                if (userFinances.isUnderTransferEmbargo())
                    cout << "  *** TRANSFER EMBARGO IN EFFECT ***\n";
                printSeparator();
                cout << "  1. Add Sponsorship Deal\n";
                cout << "  0. Back\n";
                cout << "Choice: ";
                int fc; cin >> fc;
                if (fc == 1) {
                    string sname; int64_t sval; int syears;
                    cout << "Sponsor name: "; cin.ignore(); getline(cin, sname);
                    cout << "Annual value: "; cin >> sval;
                    cout << "Years: "; cin >> syears;
                    userFinances.addSponsorshipDeal(sname, sval, syears);
                    cout << "Sponsorship deal added.\n";
                }
                break;
            }
            case 7: {
                // Staff
                printHeader("STAFF");
                if (userStaff.empty()) {
                    cout << "  No staff hired yet.\n";
                } else {
                    cout << left << setw(22) << "Name" << setw(20) << "Role"
                         << setw(8) << "Wage/m" << "Relationship\n";
                    printSeparator();
                    auto roleStr = [](StaffRole r) -> string {
                        switch(r) {
                            case StaffRole::AssistantManager:  return "Asst Manager";
                            case StaffRole::GoalkeepingCoach:  return "GK Coach";
                            case StaffRole::DefensiveCoach:    return "Def Coach";
                            case StaffRole::AttackingCoach:    return "Att Coach";
                            case StaffRole::FitnessCoach:      return "Fitness Coach";
                            case StaffRole::SetPieceCoach:     return "Set Piece Coach";
                            case StaffRole::Scout:             return "Scout";
                            case StaffRole::Physio:            return "Physio";
                            case StaffRole::HeadOfYouth:       return "Head of Youth";
                            default:                           return "Staff";
                        }
                    };
                    for (auto& s : userStaff)
                        cout << left << setw(22) << s->getName().substr(0,21)
                             << setw(20) << roleStr(s->getRole())
                             << setw(8) << s->getMonthlyWage()
                             << s->getRelationshipWithManager() << "/100\n";
                }
                printSeparator();
                cout << "  Staff affect training quality, injury recovery, scouting and youth development.\n";
                break;
            }
            case 8: {
                // Club records
                printHeader("CLUB RECORDS: " + myClub->getName());
                cout << "  Biggest Win Margin:    " << myClub->getBiggestWinMargin()      << " goals\n";
                cout << "  Heaviest Defeat:       " << myClub->getHeaviestDefeatMargin()  << " goals\n";
                cout << "  Most Points in Season: " << myClub->getMostPointsSeason()      << "\n";
                cout << "  Most Goals in Season:  " << myClub->getMostGoalsSeason()       << "\n";
                cout << "  Highest Transfer Out:  " << myClub->getHighestTransferReceived() << "\n";
                cout << "  Highest Transfer In:   " << myClub->getHighestTransferPaid()   << "\n";
                cout << "  Total Trophies:        " << myClub->getTotalTrophies()         << "\n";
                if (!myClub->getTrophyCabinet().empty()) {
                    cout << "  Trophy Cabinet:\n";
                    for (auto& t : myClub->getTrophyCabinet())
                        cout << "    " << t.year << " - " << t.competitionName << "\n";
                }
                printSeparator();
                cout << "  MANAGER RECORD: W" << userManager->getWins()
                     << " D" << userManager->getDraws()
                     << " L" << userManager->getLosses() << "\n";
                break;
            }
            case 9: {
                // H2H
                cout << "Enter opponent name: ";
                string opp; cin.ignore(); getline(cin, opp);
                HeadToHeadRecord rec = myClub->getHeadToHead(opp);
                printHeader("H2H vs " + opp);
                cout << "  W:" << rec.wins << " D:" << rec.draws << " L:" << rec.losses
                     << " | GF:" << rec.goalsFor << " GA:" << rec.goalsAgainst << "\n";
                cout << "  Biggest Win:    " << rec.biggestWinMargin   << " goals\n";
                cout << "  Biggest Defeat: " << rec.biggestDefeatMargin << " goals\n";
                if (!rec.playerStats.empty()) {
                    cout << "  Top Scorers (in this fixture):\n";
                    auto scorers = rec.getTopScorers(5);
                    for (auto& ps : scorers) {
                        auto it = allPlayers.find(ps.playerId);
                        string name = (it != allPlayers.end()) ? it->second->getName() : ps.playerId;
                        cout << "    " << name << " - " << ps.goals << " goals\n";
                    }
                }
                break;
            }
            default: cout << "Invalid choice.\n"; break;
        }
    }
}

// ========== TRANSFER MENU ==========
void showTransferMenu() {
    bool back = false;
    while (!back) {
        printHeader("TRANSFERS");
        bool windowOpen = TransferEngine::isTransferWindowOpen(calendar.getMonth());
        cout << "  Window: " << (windowOpen ? "OPEN" : "CLOSED") << "\n";
        cout << "  Transfer Budget: " << userFinances.getTransferBudget() << "\n\n";
        cout << "  1. Browse Players (by team)\n";
        cout << "  2. Make a Transfer Bid\n";
        cout << "  3. Make a Loan Offer\n";
        cout << "  4. View Active Bids\n";
        cout << "  5. View Free Agents\n";
        cout << "  0. Back\n";
        cout << "Choice: ";
        int c; cin >> c;
        switch (c) {
            case 0: back = true; break;
            case 1: {
                // Browse players by team
                cout << "Club name: "; cin.ignore(); string name; getline(cin, name);
                auto it = allTeams.find(name);
                if (it == allTeams.end()) { cout << "Club not found.\n"; break; }
                const auto& squad = it->second->getSeniorSquad();
                cout << left << setw(4) << "#" << setw(22) << "Name"
                     << setw(5) << "POS" << setw(5) << "OVR" << setw(5) << "AGE"
                     << setw(12) << "Value" << "Wage/wk\n";
                printSeparator();
                for (size_t i = 0; i < squad.size(); ++i) {
                    const auto& p = squad[i];
                    cout << left << setw(4) << (i+1)
                         << setw(22) << p->getName().substr(0,21)
                         << setw(5) << p->getPrimaryPosition()
                         << setw(5) << p->getOverall()
                         << setw(5) << p->getAge()
                         << setw(12) << TransferEngine::calculateMarketValue(p)
                         << p->getWeeklyWage() << "\n";
                }
                break;
            }
            case 2:
            case 3: {
                bool isLoan = (c == 3);
                if (!windowOpen) { cout << "Transfer window is closed.\n"; break; }
                cout << "Selling club name: "; cin.ignore(); string club; getline(cin, club);
                auto cit = allTeams.find(club);
                if (cit == allTeams.end()) { cout << "Club not found.\n"; break; }
                cout << "Player number (1-" << cit->second->getSeniorSquad().size() << "): ";
                int pi; cin >> pi;
                const auto& squad = cit->second->getSeniorSquad();
                if (pi < 1 || pi > (int)squad.size()) { cout << "Invalid.\n"; break; }
                PlayerPtr target = squad[pi - 1];
                int64_t mv = TransferEngine::calculateMarketValue(target, myClub);
                cout << "Market value: " << mv << "\n";
                cout << "Your bid amount: ";
                int64_t bid; cin >> bid;
                TransferBid tb;
                tb.buyer = myClub;
                tb.seller = cit->second;
                tb.player = target;
                tb.amount = bid;
                tb.isLoan = isLoan;
                if (isLoan) {
                    cout << "Loan months: "; cin >> tb.loanMonths;
                }
                // Simple acceptance: if bid >= 90% of market value
                if (bid >= (int64_t)(mv * 0.9)) {
                    tb.accepted = true;
                    TransferEngine::completeTransfer(tb);
                    myClub->addToSenior(target);
                    cit->second->removePlayer(target->getUniqueId());
                    userFinances.makeTransferPayment(bid);
                    myClub->updateTransferRecord(bid, false);
                    cit->second->updateTransferRecord(bid, true);
                    cout << "Transfer ACCEPTED! " << target->getName()
                         << " joins " << myClub->getName() << "!\n";
                } else {
                    tb.rejected = true;
                    cout << "Transfer REJECTED. Club wants closer to " << mv << ".\n";
                    activeBids.push_back(tb);
                }
                break;
            }
            case 4: {
                // Active bids
                printHeader("ACTIVE BIDS");
                if (activeBids.empty()) { cout << "  No active bids.\n"; break; }
                for (size_t i = 0; i < activeBids.size(); ++i) {
                    auto& tb = activeBids[i];
                    cout << i+1 << ". " << tb.player->getName()
                         << " | Bid: " << tb.amount
                         << " | Status: " << (tb.accepted ? "Accepted" : tb.rejected ? "Rejected" : "Pending")
                         << "\n";
                }
                cout << "  1. Raise bid  0. Back\nChoice: ";
                int bc; cin >> bc;
                if (bc == 1 && !activeBids.empty()) {
                    cout << "Bid number: "; int bi; cin >> bi;
                    if (bi >= 1 && bi <= (int)activeBids.size()) {
                        auto& tb = activeBids[bi-1];
                        int64_t mv = TransferEngine::calculateMarketValue(tb.player, myClub);
                        cout << "New offer (market value ~" << mv << "): ";
                        int64_t newBid; cin >> newBid;
                        TransferEngine::negotiateBid(tb, newBid);
                        if (tb.accepted) {
                            TransferEngine::completeTransfer(tb);
                            myClub->addToSenior(tb.player);
                            if (tb.seller) tb.seller->removePlayer(tb.player->getUniqueId());
                            userFinances.makeTransferPayment(newBid);
                            cout << "Transfer COMPLETED! " << tb.player->getName() << " signed!\n";
                            activeBids.erase(activeBids.begin() + (bi-1));
                        } else {
                            cout << "Bid still rejected.\n";
                        }
                    }
                }
                break;
            }
            case 5: {
                // Free agents (players with no team — contract == 0 months)
                printHeader("FREE AGENTS");
                cout << left << setw(22) << "Name" << setw(5) << "POS"
                     << setw(5) << "OVR" << setw(5) << "AGE" << "Wage/wk\n";
                printSeparator();
                int count = 0;
                for (auto& kv : allPlayers) {
                    auto& p = kv.second;
                    if (p->getContractLengthMonths() <= 0) {
                        cout << left << setw(22) << p->getName().substr(0,21)
                             << setw(5) << p->getPrimaryPosition()
                             << setw(5) << p->getOverall()
                             << setw(5) << p->getAge()
                             << p->getWeeklyWage() << "\n";
                        if (++count >= 30) { cout << "  (showing first 30)\n"; break; }
                    }
                }
                if (count == 0) cout << "  No free agents found.\n";
                break;
            }
            default: cout << "Invalid choice.\n"; break;
        }
    }
}

// ========== SCOUTING MENU ==========
void showScoutingMenu() {
    bool back = false;
    while (!back) {
        printHeader("SCOUTING");
        const auto& assignments = userScouting.getAssignments();
        cout << "  Active Assignments: " << assignments.size() << "\n\n";
        cout << "  1. View Assignments\n";
        cout << "  2. Add Assignment\n";
        cout << "  3. Remove Assignment\n";
        cout << "  4. Country Knowledge\n";
        cout << "  5. Scout a Specific Player\n";
        cout << "  0. Back\n";
        cout << "Choice: ";
        int c; cin >> c;
        switch (c) {
            case 0: back = true; break;
            case 1: {
                printHeader("SCOUT ASSIGNMENTS");
                if (assignments.empty()) { cout << "  No assignments.\n"; break; }
                for (size_t i = 0; i < assignments.size(); ++i) {
                    auto& a = assignments[i];
                    cout << i+1 << ". Country: " << a.targetCountry;
                    if (!a.targetClub.empty()) cout << " | Club: " << a.targetClub;
                    cout << " | Weeks: " << a.weeksAssigned
                         << (a.active ? " [ACTIVE]" : " [DONE]") << "\n";
                }
                break;
            }
            case 2: {
                ScoutAssignment sa;
                cout << "Country to scout: "; cin.ignore(); getline(cin, sa.targetCountry);
                cout << "Specific club (or Enter to skip): "; getline(cin, sa.targetClub);
                cout << "Weeks to assign: "; cin >> sa.weeksAssigned;
                userScouting.addAssignment(sa);
                userScouting.increaseKnowledge(sa.targetCountry, 5);
                cout << "Assignment added.\n";
                break;
            }
            case 3: {
                if (assignments.empty()) { cout << "No assignments to remove.\n"; break; }
                cout << "Assignment number to remove: "; int ri; cin >> ri;
                if (ri >= 1 && ri <= (int)assignments.size()) {
                    userScouting.removeAssignment((size_t)(ri - 1));
                    cout << "Removed.\n";
                }
                break;
            }
            case 4: {
                printHeader("COUNTRY KNOWLEDGE");
                cout << "Enter country name: "; cin.ignore(); string country; getline(cin, country);
                int know = userScouting.getKnowledge(country);
                cout << "  " << country << ": " << know << "/100\n";
                if (know < 20)      cout << "  Very limited - scouting reports will be inaccurate.\n";
                else if (know < 50) cout << "  Some knowledge - moderate accuracy.\n";
                else if (know < 80) cout << "  Good knowledge - reliable reports.\n";
                else                cout << "  Excellent knowledge - highly accurate reports.\n";
                break;
            }
            case 5: {
                // Scout a player
                cout << "Club name: "; cin.ignore(); string club; getline(cin, club);
                auto cit = allTeams.find(club);
                if (cit == allTeams.end()) { cout << "Club not found.\n"; break; }
                cout << "Player # (1-" << cit->second->getSeniorSquad().size() << "): ";
                int pi; cin >> pi;
                const auto& squad = cit->second->getSeniorSquad();
                if (pi < 1 || pi > (int)squad.size()) { cout << "Invalid.\n"; break; }
                const PlayerPtr& target = squad[pi - 1];

                // Find scout attributes (use defaults if no scout on staff)
                int judgeAbility = 10, judgePotential = 10, adaptability = 10;
                for (auto& s : userStaff) {
                    if (s->getRole() == StaffRole::Scout) {
                        judgeAbility   = s->getAttributes().judgingAbility;
                        judgePotential = s->getAttributes().judgingPotential;
                        adaptability   = s->getAttributes().adaptability;
                        break;
                    }
                }
                auto report = userScouting.generateScoutReport(target,
                    judgeAbility, judgePotential, adaptability);
                printHeader("SCOUT REPORT: " + report.playerName);
                cout << "  Est. Overall:   " << report.estimatedOverall   << "\n";
                cout << "  Est. Potential: " << report.estimatedPotential << "\n";
                cout << "  Playstyle:      " << report.estimatedPlaystyle << "\n";
                cout << "  Est. Value:     " << report.estimatedValue     << "\n";
                cout << "  Est. Wage:      " << report.estimatedWage      << "/wk\n";
                cout << "  Contract left:  " << report.contractMonthsLeft << " months\n";
                cout << "  Accuracy:       " << report.scoutAccuracy      << "%\n";
                cout << "  Verdict:        " << report.verdict            << "\n";
                // Improve knowledge of that country
                userScouting.increaseKnowledge(target->getNationality(), 2);
                break;
            }
            default: cout << "Invalid choice.\n"; break;
        }
    }
}

// ========== WORLD MENU ==========
void showWorldMenu() {
    bool back = false;
    while (!back) {
        printHeader("WORLD");
        cout << "  1. Browse Leagues\n";
        cout << "  2. Search Club\n";
        cout << "  3. Top Scorers (Global)\n";
        cout << "  4. World History / Awards\n";
        cout << "  0. Back\n";
        cout << "Choice: ";
        int c; cin >> c;
        switch (c) {
            case 0: back = true; break;
            case 1: {
                for (size_t i = 0; i < allLeagues.size(); ++i)
                    cout << i+1 << ". " << allLeagues[i]->getName()
                         << " (" << allLeagues[i]->getCountry() << ")\n";
                cout << "Select league (0 to cancel): "; int li; cin >> li;
                if (li >= 1 && li <= (int)allLeagues.size())
                    allLeagues[li-1]->printTable();
                break;
            }
            case 2: {
                cout << "Club name: "; cin.ignore(); string name; getline(cin, name);
                auto it = allTeams.find(name);
                if (it == allTeams.end()) { cout << "Not found.\n"; break; }
                auto& t = it->second;
                printHeader("CLUB: " + t->getName());
                cout << "  Country:    " << t->getCountry()    << "\n";
                cout << "  Level:      " << t->getClubLevel()  << "\n";
                cout << "  Reputation: " << t->getReputation() << "\n";
                cout << "  Founded:    " << t->getFoundedYear()<< "\n";
                cout << "  Stadium:    " << t->getStadiumName()<< "\n";
                cout << "  Squad Size: " << t->getSeniorSquad().size() << "\n";
                cout << "  Season: W" << t->getWins() << " D" << t->getDraws()
                     << " L" << t->getLosses() << " GD"
                     << t->getGoalDifference() << " Pts" << t->getPoints() << "\n";
                break;
            }
            case 3: {
                // Top scorers
                printHeader("TOP SCORERS (GLOBAL)");
                vector<pair<int, PlayerPtr>> scorers;
                for (auto& kv : allPlayers)
                    if (kv.second->getGoals() > 0)
                        scorers.push_back({kv.second->getGoals(), kv.second});
                sort(scorers.begin(), scorers.end(), [](auto& a, auto& b){ return a.first > b.first; });
                cout << left << setw(4) << "#" << setw(22) << "Name"
                     << setw(5) << "Gls" << setw(5) << "Ast" << "Team\n";
                printSeparator();
                int limit = min(25, (int)scorers.size());
                for (int i = 0; i < limit; ++i) {
                    auto& p = scorers[i].second;
                    // Find their team
                    string team = "?";
                    for (auto& kv : allTeams)
                        for (auto& pl : kv.second->getSeniorSquad())
                            if (pl->getUniqueId() == p->getUniqueId()) { team = kv.first; break; }
                    cout << left << setw(4) << (i+1)
                         << setw(22) << p->getName().substr(0,21)
                         << setw(5) << p->getGoals()
                         << setw(5) << p->getAssists()
                         << team.substr(0, 20) << "\n";
                }
                break;
            }
            case 4: {
                printHeader("WORLD HISTORY");
                // WorldHistory doesn't have direct getters exposed here, just inform user
                cout << "  Season awards are saved to history.json after each season ends.\n";
                cout << "  Check the file for detailed historical records.\n";
                break;
            }
            default: cout << "Invalid choice.\n"; break;
        }
    }
}

// ========== INTERNATIONAL MENU ==========
void showInternationalMenu() {
    bool back = false;
    while (!back) {
        printHeader("INTERNATIONAL FOOTBALL");
        cout << "  Year: " << internationalManager.getCurrentYear() << "\n";
        cout << "  1. View National Team\n";
        cout << "  2. World Cup Qualifying Table\n";
        cout << "  3. Euros Qualifying Table\n";
        cout << "  4. Tournament Group Standings\n";
        cout << "  5. International Break Info\n";
        cout << "  0. Back\n";
        cout << "Choice: ";
        int c; cin >> c;
        switch (c) {
            case 0: back = true; break;
            case 1: {
                cout << "Country: "; cin.ignore(); string country; getline(cin, country);
                NationalTeam* nt = internationalManager.getNationalTeam(country);
                if (!nt) { cout << "National team not found.\n"; break; }
                printHeader("NATIONAL TEAM: " + country);
                cout << "  World Ranking: " << nt->worldRanking << "\n";
                cout << "  Players selected: " << nt->playerIds.size() << "\n";
                // Show top players
                cout << "  Key players:\n";
                int shown = 0;
                for (auto& pid : nt->playerIds) {
                    auto it = allPlayers.find(pid);
                    if (it != allPlayers.end() && shown < 10) {
                        cout << "    " << it->second->getName()
                             << " (" << it->second->getPrimaryPosition() << " "
                             << it->second->getOverall() << ")\n";
                        ++shown;
                    }
                }
                break;
            }
            case 2: {
                // World Cup qualifying - show country points from internationalManager
                printHeader("WORLD CUP QUALIFYING");
                const auto& qf = internationalManager.getQualifyingFixtures(InternationalTournament::WorldCup);
                if (qf.empty()) {
                    cout << "  No qualifying fixtures scheduled yet.\n";
                } else {
                    cout << "  " << qf.size() << " qualifying fixtures scheduled.\n";
                    cout << "  Use Advance Day to simulate qualifying matches.\n";
                    // Show last few played
                    int shown = 0;
                    for (auto it = qf.rbegin(); it != qf.rend() && shown < 10; ++it) {
                        if (it->played) {
                            cout << "  " << it->homeCountry << " " << it->homeGoals
                                 << "-" << it->awayGoals << " " << it->awayCountry
                                 << " [" << it->group << " MD" << it->matchday << "]\n";
                            ++shown;
                        }
                    }
                }
                break;
            }
            case 3: {
                printHeader("EUROS QUALIFYING");
                const auto& qf = internationalManager.getQualifyingFixtures(InternationalTournament::UEFAEuros);
                if (qf.empty()) {
                    cout << "  No Euros qualifying fixtures scheduled.\n";
                } else {
                    cout << "  " << qf.size() << " fixtures scheduled.\n";
                    int shown = 0;
                    for (auto it = qf.rbegin(); it != qf.rend() && shown < 10; ++it) {
                        if (it->played) {
                            cout << "  " << it->homeCountry << " " << it->homeGoals
                                 << "-" << it->awayGoals << " " << it->awayCountry
                                 << " [" << it->group << "]\n";
                            ++shown;
                        }
                    }
                }
                break;
            }
            case 4: {
                cout << "Tournament (1=World Cup, 2=Euros, 3=Copa America, 4=AFCON): ";
                int ti; cin >> ti;
                cout << "Group (e.g. A): "; string group; cin >> group;
                InternationalTournament t;
                switch (ti) {
                    case 2:  t = InternationalTournament::UEFAEuros;    break;
                    case 3:  t = InternationalTournament::CopaAmerica;  break;
                    case 4:  t = InternationalTournament::AFCON;        break;
                    default: t = InternationalTournament::WorldCup;     break;
                }
                auto standings = internationalManager.getGroupStandings(t, "Group " + group);
                if (standings.empty()) {
                    cout << "No standings found for Group " << group << ".\n";
                } else {
                    printHeader("GROUP " + group + " STANDINGS");
                    vector<pair<string,int>> sorted(standings.begin(), standings.end());
                    sort(sorted.begin(), sorted.end(), [](auto& a, auto& b){ return a.second > b.second; });
                    cout << left << setw(4) << "#" << setw(22) << "Country" << "Pts\n";
                    printSeparator();
                    for (size_t i = 0; i < sorted.size(); ++i)
                        cout << left << setw(4) << (i+1) << setw(22) << sorted[i].first
                             << sorted[i].second << "\n";
                }
                break;
            }
            case 5: {
                bool intlBreak = internationalManager.isInternationalBreak(calendar.getDateString());
                cout << "\n  International break on " << calendar.getDateString()
                     << ": " << (intlBreak ? "YES" : "No") << "\n";
                if (myClub && intlBreak) {
                    auto country = myClub->getCountry();
                    auto calledUp = internationalManager.getCalledUpPlayers(country);
                    cout << "  Called-up from " << myClub->getName() << ": ";
                    int found = 0;
                    for (auto& pid : calledUp) {
                        auto pit = allPlayers.find(pid);
                        if (pit != allPlayers.end()) {
                            for (auto& p : myClub->getSeniorSquad()) {
                                if (p->getUniqueId() == pid) {
                                    cout << p->getName() << "  ";
                                    ++found;
                                    break;
                                }
                            }
                        }
                    }
                    if (found == 0) cout << "None";
                    cout << "\n";
                }
                break;
            }
            default: cout << "Invalid choice.\n"; break;
        }
    }
}

// ========== EDITOR MENU ==========
void showEditorMenu() {
    printHeader("IN-GAME EDITOR");
    cout << "  Editor status: " << (Editor::isEnabled ? "ENABLED" : "DISABLED") << "\n";
    cout << "  1. Toggle Editor\n";
    if (Editor::isEnabled) {
        cout << "  2. Edit Player (from your squad)\n";
        cout << "  3. Heal All Your Players\n";
        cout << "  4. Boost Youth Squad\n";
        cout << "  5. Edit Club Finances\n";
        cout << "  6. Edit Club Level/Reputation\n";
        cout << "  7. Undo Last Action\n";
    }
    cout << "  0. Back\n";
    cout << "Choice: ";
    int c; cin >> c;
    switch (c) {
        case 0: break;
        case 1:
            Editor::toggle();
            cout << "Editor " << (Editor::isEnabled ? "ENABLED" : "DISABLED") << ".\n";
            break;
        case 2: {
            if (!Editor::isEnabled) { cout << "Enable editor first.\n"; break; }
            if (!myClub) { cout << "No club.\n"; break; }
            auto& squad = myClub->getSeniorSquad();
            for (size_t i = 0; i < squad.size(); ++i)
                cout << i+1 << ". " << squad[i]->getName()
                     << " OVR:" << squad[i]->getOverall() << "\n";
            cout << "Player #: "; int pi; cin >> pi;
            if (pi < 1 || pi > (int)squad.size()) { cout << "Invalid.\n"; break; }
            cout << "1.Shooting 2.Passing 3.Dribbling 4.Tackling 5.Pace 6.Stamina 7.Vision\n";
            cout << "Attribute: "; int ai; cin >> ai;
            cout << "New value (1-20): "; int nv; cin >> nv;
            Editor::editPlayerAttribute(squad[pi-1], ai - 1, nv);
            cout << "Attribute updated.\n";
            break;
        }
        case 3: {
            if (!Editor::isEnabled) { cout << "Enable editor first.\n"; break; }
            if (!myClub) { cout << "No club.\n"; break; }
            Editor::healAllPlayers(myClub->getSeniorSquad());
            cout << "All players healed.\n";
            break;
        }
        case 4: {
            if (!Editor::isEnabled) { cout << "Enable editor first.\n"; break; }
            if (!myClub) { cout << "No club.\n"; break; }
            cout << "Boost amount (1-5): "; int ba; cin >> ba;
            Editor::boostYouth(myClub->getYouthSquad(), max(1, min(5, ba)));
            cout << "Youth squad boosted.\n";
            break;
        }
        case 5: {
            if (!Editor::isEnabled) { cout << "Enable editor first.\n"; break; }
            if (!myClub) { cout << "No club.\n"; break; }
            cout << "New transfer budget: "; int64_t tb; cin >> tb;
            userFinances.setTransferBudget(tb);
            Editor::editClubFinances(myClub, tb);
            cout << "Budget set.\n";
            break;
        }
        case 6: {
            if (!Editor::isEnabled) { cout << "Enable editor first.\n"; break; }
            if (!myClub) { cout << "No club.\n"; break; }
            cout << "New club level (1-20): "; int lv; cin >> lv;
            Editor::editClubLevel(myClub, max(1, min(20, lv)));
            cout << "New reputation (1-100): "; int rep; cin >> rep;
            Editor::editClubReputation(myClub, max(1, min(100, rep)));
            cout << "Club updated.\n";
            break;
        }
        case 7: {
            if (!Editor::isEnabled) { cout << "Enable editor first.\n"; break; }
            Editor::undo();
            cout << "Undo applied.\n";
            break;
        }
        default: cout << "Invalid choice.\n"; break;
    }
}

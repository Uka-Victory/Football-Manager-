// main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <fstream>
#include "Utils.hpp"
#include "WorldData.hpp"
#include "NamePool.hpp"
#include "TeamGenerator.hpp"
#include "League.hpp"
#include "GameCalendar.hpp"
#include "TransferEngine.hpp"
#include "TrainingEngine.hpp"
#include "MatchEngine.hpp"
#include "Manager.hpp"
#include "WorldHistory.hpp"
#include "DataHub.hpp"
#include "ContinentalManager.hpp"
#include "InternationalManager.hpp"
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

const string SAVE_FILE = "save_game.json";

// ========== FORWARD DECLARATIONS ==========
void generateWorld();
void selectClub();
void advanceDay();
void saveGame();
void loadGame();
void showMainMenu();
void showMyClubMenu();
void showWorldMenu();

// ========== MAIN ==========
int main() {
    cout << "=== FOOTBALL MANAGER C++ ===\n";
    userManager = make_shared<Manager>("Your Name", 40, 50);

    cout << "1. New Game\n2. Load Game\nChoice: ";
    int c;
    cin >> c;
    if (c == 2) {
        loadGame();
        if (!myClub) {
            cout << "Load failed, starting new game.\n";
            generateWorld();
            selectClub();
        }
    } else {
        generateWorld();
        selectClub();
        saveGame();
    }

    bool running = true;
    while (running) {
        showMainMenu();
        int choice;
        cin >> choice;
        switch (choice) {
            case 1: advanceDay(); break;
            case 2: showMyClubMenu(); break;
            case 3: showWorldMenu(); break;
            case 4: saveGame(); break;
            case 5: running = false; break;
            default: cout << "Invalid choice.\n";
        }
    }
    cout << "Thanks for playing.\n";
    return 0;
}

// ========== WORLD GENERATION ==========
void generateWorld() {
    worldData.load("world_data.json");
    namePool.load("countries_data.txt");
    teamGenerator = TeamGenerator(namePool);

    // Collect all country names for national teams
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
    cout << "Generated " << allLeagues.size() << " leagues with " << allTeams.size() << " clubs.\n";

    // Generate national teams
    internationalManager.generateNationalTeams(allPlayers, allCountryNames);

    // Set up initial continental competitions (qualification from current season's leagues)
    // For new game, we can delay until first season end, or set up with placeholders.
}

// ========== CLUB SELECTION ==========
void selectClub() {
    cout << "\nChoose a country:\n";
    const auto& countries = worldData.getBaseCountries();
    for (size_t i = 0; i < countries.size(); ++i) cout << i + 1 << ". " << countries[i].name << "\n";
    int ci; cin >> ci;
    if (ci < 1 || ci > (int)countries.size()) { cout << "Invalid, defaulting to first country.\n"; ci = 1; }
    const auto& country = countries[ci - 1];

    cout << "\nChoose a league:\n";
    for (size_t i = 0; i < country.leagues.size(); ++i) cout << i + 1 << ". " << country.leagues[i].name << "\n";
    int li; cin >> li;
    if (li < 1 || li > (int)country.leagues.size()) { cout << "Invalid, defaulting to first league.\n"; li = 1; }

    LeaguePtr chosenLeague = nullptr;
    for (auto& league : allLeagues) {
        if (league->getName() == country.leagues[li - 1].name && league->getCountry() == country.name) {
            chosenLeague = league;
            break;
        }
    }
    if (!chosenLeague) { cerr << "League not found.\n"; return; }

    cout << "\nChoose a club:\n";
    const auto& teams = chosenLeague->getTeams();
    for (size_t i = 0; i < teams.size(); ++i) cout << i + 1 << ". " << teams[i]->getName() << "\n";
    int ti; cin >> ti;
    if (ti < 1 || ti > (int)teams.size()) { cout << "Invalid, defaulting to first club.\n"; ti = 1; }
    myClub = teams[ti - 1];
    userManager->setCurrentClubName(myClub->getName());
    cout << "\nYou are now managing " << myClub->getName() << "!\n";
}

// ========== ADVANCE DAY ==========
void advanceDay() {
    string today = calendar.getDateString();

    // 1. Process all fixtures (domestic leagues + continental)
    // Domestic leagues
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
                fix.homeGoals, fix.awayGoals,
                fix.homeTeam->getSeniorSquad(), res.playerStats);
            fix.awayTeam->updateHeadToHead(fix.homeTeam->getName(),
                fix.awayGoals, fix.homeGoals,
                fix.awayTeam->getSeniorSquad(), res.playerStats);
            dataHub.ingestMatch(fix, res);
            cout << fix.homeTeam->getName() << " " << fix.homeGoals << " - "
                 << fix.awayGoals << " " << fix.awayTeam->getName() << "\n";
        }
    }

    // Continental fixtures
    for (auto& comp : {"UCL", "UEL", "UECL"}) {
        const auto& contFixtures = continentalManager.getLeaguePhaseFixtures(comp);
        for (auto& fix : contFixtures) {
            if (fix.date == today && !fix.played) {
                auto res = MatchEngine::simulateMatch(fix.homeTeam, fix.awayTeam);
                const_cast<Fixture&>(fix).homeGoals = res.homeGoals;
                const_cast<Fixture&>(fix).awayGoals = res.awayGoals;
                const_cast<Fixture&>(fix).played = true;
                // Update continental points/GD table (done inside ContinentalManager's record methods, but we call later)
                // For now, the result is stored; coefficient update happens at season end.
                cout << "[CONT] " << fix.homeTeam->getName() << " " << res.homeGoals << " - "
                     << res.awayGoals << " " << fix.awayTeam->getName() << "\n";
            }
        }
    }

    // International qualifying / tournament fixtures
    for (auto tournament : {InternationalTournament::WorldCup, InternationalTournament::UEFAEuros,
                            InternationalTournament::CopaAmerica, InternationalTournament::AFCON}) {
        for (const auto& qfix : internationalManager.getQualifyingFixtures(tournament)) {
            if (qfix.date == today && !qfix.played) {
                // Simulate using a simple random score (since we don't have full rosters in QualifyingFixture)
                int homeG = Utils::randInt(0, 3);
                int awayG = Utils::randInt(0, 3);
                // Record result
                internationalManager.recordTournamentResult(tournament, qfix.homeCountry, qfix.awayCountry,
                                                            homeG, awayG, today);
                cout << "[INTL] " << qfix.homeCountry << " " << homeG << " - " << awayG << " " << qfix.awayCountry << "\n";
            }
        }
        for (const auto& tfix : internationalManager.getTournamentFixtures(tournament)) {
            if (tfix.date == today && !tfix.played) {
                int homeG = Utils::randInt(0, 3);
                int awayG = Utils::randInt(0, 3);
                internationalManager.recordTournamentResult(tournament, tfix.homeCountry, tfix.awayCountry,
                                                            homeG, awayG, today);
                cout << "[INTL FINAL] " << tfix.homeCountry << " " << homeG << " - " << awayG << " " << tfix.awayCountry << "\n";
            }
        }
    }

    // 2. Training recovery
    for (auto& league : allLeagues)
        for (auto& team : league->getTeams())
            TrainingEngine::processDailyTraining(team);

    // 3. Injury/suspension recovery
    for (auto& kv : allPlayers) {
        kv.second->recoverDay();
        kv.second->reduceSuspension();
        kv.second->reduceContract();
    }

    // 4. Weekly financial processing (Sunday)
    if (calendar.getWeekday() == 0) {
        // placeholder for Finances::processWeekly
    }

    // 5. End‑of‑season
    if (calendar.isSeasonEnd()) {
        cout << "\n===== END OF SEASON =====\n";

        // League end‑of‑season + promotion/relegation
        for (size_t i = 0; i < allLeagues.size(); ++i) {
            League* lower = (i + 1 < allLeagues.size()) ? allLeagues[i + 1].get() : nullptr;
            allLeagues[i]->endSeason(lower);
        }

        // Awards & history
        worldHistory->computeSeasonAwards(allLeagues, allPlayers);
        worldHistory->saveToFile("history.json");

        // Update country & club coefficients
        continentalManager.updateCountryCoefficients(allLeagues, allTeams);
        continentalManager.updateClubCoefficients(allTeams);

        // Set up next season's continental competitions
        vector<string> uclQualified = continentalManager.getQualifiedClubs("UCL", allLeagues, calendar.getYear());
        vector<string> uelQualified = continentalManager.getQualifiedClubs("UEL", allLeagues, calendar.getYear());
        vector<string> ueclQualified = continentalManager.getQualifiedClubs("UECL", allLeagues, calendar.getYear());
        continentalManager.setupChampionsLeague(uclQualified, allTeams, calendar);
        continentalManager.setupEuropaLeague(uelQualified, allTeams, calendar);
        continentalManager.setupConferenceLeague(ueclQualified, allTeams, calendar);

        // Youth intake
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

        // Generate next season fixtures
        for (auto& league : allLeagues)
            league->generateSchedule(calendar.getYear() + 1, 8, 1);

        // Reset data hub
        dataHub.resetSeason();

        // Set up international qualifiers for the new cycle if needed
        int nextYear = calendar.getYear() + 1;
        internationalManager.setYear(nextYear);
        if (internationalManager.isTournamentYear(InternationalTournament::WorldCup)) {
            internationalManager.setupWorldCupQualifiers(calendar);
        }
        if (internationalManager.isTournamentYear(InternationalTournament::UEFAEuros)) {
            internationalManager.setupEurosQualifiers(calendar);
        }
    }

    // 6. Advance calendar
    calendar.advanceOneDay();
}

// ========== SAVE / LOAD ==========
void saveGame() {
    json j;
    j["calendar"] = calendar.toJson();
    j["userManager"] = userManager->toJson();
    j["myClubName"] = myClub ? myClub->getName() : "";
    j["leagues"] = json::array();
    for (auto& l : allLeagues) j["leagues"].push_back(l->toJson());
    j["teams"] = json::array();
    for (auto& kv : allTeams) j["teams"].push_back(kv.second->toJson());
    j["players"] = json::array();
    for (auto& kv : allPlayers) {
        json pj;
        Player& p = *kv.second;
        nlohmann::adl_serializer<Player>::to_json(pj, p);
        j["players"].push_back(pj);
    }
    j["worldHistory"] = worldHistory->toJson();
    j["continentalManager"] = continentalManager.toJson();
    j["internationalManager"] = internationalManager.toJson();
    ofstream out(SAVE_FILE);
    out << j.dump(2);
    cout << "Game saved.\n";
}

void loadGame() {
    ifstream in(SAVE_FILE);
    if (!in) { cout << "No save file.\n"; return; }
    json j;
    in >> j;

    calendar.fromJson(j["calendar"]);
    userManager = make_shared<Manager>();
    userManager->fromJson(j["userManager"]);

    // Reconstruct all players
    allPlayers.clear();
    if (j.contains("players")) {
        for (auto& pj : j["players"]) {
            auto p = make_shared<Player>();
            nlohmann::adl_serializer<Player>::from_json(pj, *p);
            allPlayers[p->getUniqueId()] = p;
        }
    }

    // Reconstruct teams
    allTeams.clear();
    if (j.contains("teams")) {
        for (auto& tj : j["teams"]) {
            auto team = Team::fromJson(tj);
            // Replace placeholder players with real ones from allPlayers
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
    }

    // Reconstruct leagues
    allLeagues.clear();
    if (j.contains("leagues")) {
        for (auto& lj : j["leagues"]) {
            auto league = League::fromJson(lj, allTeams);
            allLeagues.push_back(league);
        }
    }

    string myClubName = j.value("myClubName", "");
    myClub = allTeams.count(myClubName) ? allTeams[myClubName] : nullptr;
    if (!myClub && !allTeams.empty()) myClub = allTeams.begin()->second;

    worldHistory->fromJson(j.value("worldHistory", json::object()));
    continentalManager.fromJson(j.value("continentalManager", json::object()));
    internationalManager.fromJson(j.value("internationalManager", json::object()));

    cout << "Game loaded.\n";
}

// ========== MENUS ==========
void showMainMenu() {
    cout << "\n--- Date: " << calendar.getDateString() << " ---\n";
    cout << "1. Advance Day\n";
    cout << "2. My Club\n";
    cout << "3. World\n";
    cout << "4. Save\n";
    cout << "5. Quit\n";
    cout << "Choice: ";
}

void showMyClubMenu() {
    if (!myClub) { cout << "No club selected.\n"; return; }
    cout << "\n=== " << myClub->getName() << " ===\n";
    cout << "1. View Squad\n";
    cout << "2. View Fixtures\n";
    cout << "3. View League Table\n";
    cout << "4. View Club Records\n";
    cout << "5. View Head‑to‑Head\n";
    cout << "0. Back\n";
    int c; cin >> c;
    switch (c) {
        case 1: {
            auto& squad = myClub->getSeniorSquad();
            for (auto& p : squad)
                cout << p->getName() << " | " << p->getPrimaryPosition() << " | OVR " << p->getOverall() << "\n";
            break;
        }
        case 2: {
            for (auto& l : allLeagues) {
                if (find(l->getTeams().begin(), l->getTeams().end(), myClub) != l->getTeams().end()) {
                    for (auto& kv : l->getFixtures()) {
                        for (auto& f : kv.second) {
                            if (f.homeTeam == myClub || f.awayTeam == myClub)
                                cout << kv.first << " " << f.homeTeam->getName()
                                     << " vs " << f.awayTeam->getName()
                                     << (f.played ? " (" + to_string(f.homeGoals) + "-" + to_string(f.awayGoals) + ")" : "") << "\n";
                        }
                    }
                }
            }
            break;
        }
        case 3: {
            for (auto& l : allLeagues) {
                if (find(l->getTeams().begin(), l->getTeams().end(), myClub) != l->getTeams().end()) {
                    l->printTable();
                    break;
                }
            }
            break;
        }
        case 4: {
            cout << "Biggest Win: " << myClub->getBiggestWinMargin() << " goals\n";
            cout << "Heaviest Defeat: " << myClub->getHeaviestDefeatMargin() << " goals\n";
            cout << "Total Trophies: " << myClub->getTotalTrophies() << "\n";
            break;
        }
        case 5: {
            string opponent;
            cout << "Opponent name: ";
            cin.ignore(); getline(cin, opponent);
            HeadToHeadRecord rec = myClub->getHeadToHead(opponent);
            cout << "W:" << rec.wins << " D:" << rec.draws << " L:" << rec.losses
                 << " GF:" << rec.goalsFor << " GA:" << rec.goalsAgainst << "\n";
            break;
        }
        default: break;
    }
}

void showWorldMenu() {
    cout << "\n=== WORLD ===\n";
    cout << "1. Browse Leagues\n";
    cout << "2. Search Club\n";
    cout << "0. Back\n";
    int c; cin >> c;
    if (c == 1) {
        for (size_t i = 0; i < allLeagues.size(); ++i) {
            cout << i + 1 << ". " << allLeagues[i]->getName()
                 << " (" << allLeagues[i]->getCountry() << ")\n";
        }
        cout << "Select league: ";
        int li; cin >> li;
        if (li >= 1 && li <= (int)allLeagues.size()) {
            allLeagues[li - 1]->printTable();
        }
    } else if (c == 2) {
        string name;
        cout << "Club name: ";
        cin.ignore();
        getline(cin, name);
        auto it = allTeams.find(name);
        if (it != allTeams.end()) {
            auto& t = it->second;
            cout << t->getName() << " | Level " << t->getClubLevel() << "\n";
            cout << "Squad size: " << t->getSeniorSquad().size() << "\n";
        } else {
            cout << "Club not found.\n";
        }
    }
}
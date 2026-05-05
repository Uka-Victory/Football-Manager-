// Player.hpp
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include "json.hpp"

using json = nlohmann::json;

// ========== PLAYSTYLES (19) ==========
enum class Playstyle {
    DeepLyingPlaymaker,    // DM/CM
    BoxToBox,              // CM
    AnchorMan,             // DM/CB
    TargetMan,             // ST
    Poacher,               // ST
    False9,                // ST/AM
    InsideForward,         // LW/RW/AM
    Raumdeuter,            // LW/RW/AM
    WideWinger,            // LW/RW
    AdvancedPlaymaker,     // AM/CM
    DeepLyingForward,      // ST/AM
    DefensiveFullBack,     // LB/RB
    AttackingFullBack,     // LB/RB
    InvertedWingBack,      // LB/RB
    BallPlayingDefender,   // CB
    Sweeper,               // CB
    Stopper,               // CB
    SweeperKeeper,         // GK
    TraditionalGoalkeeper  // GK
};

// ========== TRAITS (20) ==========
enum class Trait {
    FinesseShot,
    PowerShot,
    LongShotTaker,
    CurlsBall,
    DivesIntoTackles,
    StaysOnFeet,
    EarlyCrosser,
    TriesKillerBalls,
    PlaysShortSimplePasses,
    GiantThrowIn,
    InjuryProne,
    Flair,
    TeamPlayer,
    Selfish,
    Consistent,
    Inconsistent,
    PenaltySpecialist,
    SetPieceSpecialist,
    OneClubMan,
    BigMatchPlayer
};

// ========== SQUAD STATUS ==========
enum class SquadStatus {
    StarPlayer = 20,
    ImportantPlayer = 16,
    SquadPlayer = 11,
    Backup = 6,
    EmergencyProspect = 2
};

// ========== POSITION GROUPS ==========
enum class PositionGroup {
    GK, CB, LB, RB, DM, CM, LM, RM, AM, LW, RW, ST
};

// ========== PLAYER CLASS ==========
class Player {
private:
    friend struct nlohmann::adl_serializer<Player>;
    std::string m_uniqueId;
    std::string m_name;
    int m_age = 0;
    std::string m_nationality;
    std::string m_primaryPosition;    // "GK","CB","LB","RB","DM","CM","LM","RM","AM","LW","RW","ST"
    std::string m_secondaryPosition;

    // 17 visible attributes (1-20)
    int m_shooting = 10;
    int m_passing = 10;
    int m_crossing = 10;
    int m_dribbling = 10;
    int m_tackling = 10;
    int m_heading = 10;
    int m_positioning = 10;
    int m_composure = 10;
    int m_vision = 10;
    int m_workRate = 10;
    int m_leadership = 10;
    int m_decisions = 10;
    int m_pace = 10;
    int m_acceleration = 10;
    int m_stamina = 10;
    int m_strength = 10;
    int m_agility = 10;

    // Hidden (1-20)
    int m_injuryProneness = 10;       // lower = less prone
    int m_versatility = 10;           // higher = better in secondary positions

    // Potential (1-20)
    int m_potentialCeiling = 14;

    // Playstyle & traits
    Playstyle m_playstyle = Playstyle::BoxToBox;
    std::vector<Trait> m_traits;

    // Status
    SquadStatus m_squadStatus = SquadStatus::SquadPlayer;
    int m_fitness = 100;
    int m_morale = 70;
    int m_form = 50;

    // Injury & suspension
    bool m_injured = false;
    int m_injuryDaysRemaining = 0;
    bool m_suspended = false;
    int m_suspensionDaysRemaining = 0;

    // Contract
    int m_contractLengthMonths = 24;
    int m_weeklyWage = 5000;
    std::string m_homeGrownNation;

    // Season stats
    int m_appearances = 0;
    int m_goals = 0;
    int m_assists = 0;
    int m_yellowCards = 0;
    int m_redCards = 0;
    int m_cleanSheets = 0;
    double m_avgRating = 0.0;
    std::vector<double> m_last5Ratings;

    // Career stats
    int m_careerApps = 0, m_careerGoals = 0, m_careerAssists = 0;
    int m_careerYellowCards = 0, m_careerRedCards = 0, m_careerCleanSheets = 0;
    int m_trophiesWon = 0;
    double m_careerAvgRating = 0.0;

public:
    Player() = default;
    Player(const std::string& id) : m_uniqueId(id) {}

    // ========== GETTERS ==========
    const std::string& getUniqueId()    const { return m_uniqueId; }
    const std::string& getName()        const { return m_name; }
    int getAge()                        const { return m_age; }
    const std::string& getNationality() const { return m_nationality; }
    const std::string& getPrimaryPosition()   const { return m_primaryPosition; }
    const std::string& getSecondaryPosition() const { return m_secondaryPosition; }

    int getShooting()     const { return m_shooting; }
    int getPassing()      const { return m_passing; }
    int getCrossing()     const { return m_crossing; }
    int getDribbling()    const { return m_dribbling; }
    int getTackling()     const { return m_tackling; }
    int getHeading()      const { return m_heading; }
    int getPositioning()  const { return m_positioning; }
    int getComposure()    const { return m_composure; }
    int getVision()       const { return m_vision; }
    int getWorkRate()     const { return m_workRate; }
    int getLeadership()   const { return m_leadership; }
    int getDecisions()    const { return m_decisions; }
    int getPace()         const { return m_pace; }
    int getAcceleration() const { return m_acceleration; }
    int getStamina()      const { return m_stamina; }
    int getStrength()     const { return m_strength; }
    int getAgility()      const { return m_agility; }
    int getInjuryProneness() const { return m_injuryProneness; }
    int getVersatility()     const { return m_versatility; }
    int getPotentialCeiling() const { return m_potentialCeiling; }
    void setPotentialCeiling(int v) { m_potentialCeiling = std::max(1, std::min(20, v)); }

    Playstyle getPlaystyle() const { return m_playstyle; }
    const std::vector<Trait>& getTraits() const { return m_traits; }
    bool hasTrait(Trait t) const {
        return std::find(m_traits.begin(), m_traits.end(), t) != m_traits.end();
    }

    SquadStatus getSquadStatus() const { return m_squadStatus; }
    int getFitness()  const { return m_fitness; }
    int getMorale()   const { return m_morale; }
    int getForm()     const { return m_form; }
    bool isInjured()  const { return m_injured; }
    int getInjuryDaysRemaining() const { return m_injuryDaysRemaining; }
    bool isSuspended() const { return m_suspended; }
    int getSuspensionDaysRemaining() const { return m_suspensionDaysRemaining; }
    bool isAvailable() const { return !m_injured && !m_suspended; }

    int getContractLengthMonths() const { return m_contractLengthMonths; }
    int getWeeklyWage() const { return m_weeklyWage; }
    const std::string& getHomeGrownNation() const { return m_homeGrownNation; }

    int getAppearances()  const { return m_appearances; }
    int getGoals()        const { return m_goals; }
    int getAssists()      const { return m_assists; }
    int getYellowCards()  const { return m_yellowCards; }
    int getRedCards()     const { return m_redCards; }
    int getCleanSheets()  const { return m_cleanSheets; }
    double getAvgRating() const { return m_avgRating; }
    const std::vector<double>& getLast5Ratings() const { return m_last5Ratings; }

    int getCareerApps()    const { return m_careerApps; }
    int getCareerGoals()   const { return m_careerGoals; }
    int getCareerAssists() const { return m_careerAssists; }
    int getCareerCleanSheets() const { return m_careerCleanSheets; }
    double getCareerAvgRating() const { return m_careerAvgRating; }
    int getTrophiesWon()   const { return m_trophiesWon; }

    // Derived
    int getOverall() const {
        return (m_shooting + m_passing + m_crossing + m_dribbling + m_tackling +
                m_heading + m_positioning + m_composure + m_vision + m_workRate +
                m_leadership + m_decisions + m_pace + m_acceleration + m_stamina +
                m_strength + m_agility) / 17;
    }

    // ========== SETTERS ==========
    void setName(const std::string& n)       { m_name = n; }
    void setAge(int a)                        { m_age = a; }
    void setNationality(const std::string& n) { m_nationality = n; }
    void setPrimaryPosition(const std::string& p)   { m_primaryPosition = p; }
    void setSecondaryPosition(const std::string& p) { m_secondaryPosition = p; }
    void setHomeGrownNation(const std::string& n)   { m_homeGrownNation = n; }

    void setShooting(int v)     { m_shooting = clamp(v); }
    void setPassing(int v)      { m_passing = clamp(v); }
    void setCrossing(int v)     { m_crossing = clamp(v); }
    void setDribbling(int v)    { m_dribbling = clamp(v); }
    void setTackling(int v)     { m_tackling = clamp(v); }
    void setHeading(int v)      { m_heading = clamp(v); }
    void setPositioning(int v)  { m_positioning = clamp(v); }
    void setComposure(int v)    { m_composure = clamp(v); }
    void setVision(int v)       { m_vision = clamp(v); }
    void setWorkRate(int v)     { m_workRate = clamp(v); }
    void setLeadership(int v)   { m_leadership = clamp(v); }
    void setDecisions(int v)    { m_decisions = clamp(v); }
    void setPace(int v)         { m_pace = clamp(v); }
    void setAcceleration(int v) { m_acceleration = clamp(v); }
    void setStamina(int v)      { m_stamina = clamp(v); }
    void setStrength(int v)     { m_strength = clamp(v); }
    void setAgility(int v)      { m_agility = clamp(v); }
    void setInjuryProneness(int v) { m_injuryProneness = clamp(v); }
    void setVersatility(int v)     { m_versatility = clamp(v); }

    void setPlaystyle(Playstyle p) { m_playstyle = p; }
    void addTrait(Trait t) { if (!hasTrait(t)) m_traits.push_back(t); }
    void removeTrait(Trait t) {
        auto it = std::find(m_traits.begin(), m_traits.end(), t);
        if (it != m_traits.end()) m_traits.erase(it);
    }
    void clearTraits() { m_traits.clear(); }

    void setSquadStatus(SquadStatus s) { m_squadStatus = s; }
    void setFitness(int f)  { m_fitness = std::max(1, std::min(100, f)); }
    void setMorale(int m)   { m_morale = std::max(1, std::min(100, m)); }
    void setContractLengthMonths(int m) { m_contractLengthMonths = m; }
    void setWeeklyWage(int w) { m_weeklyWage = w; }

    // ========== ACTIONS ==========
    void recordGoal()      { m_goals++; m_careerGoals++; }
    void recordAssist()    { m_assists++; m_careerAssists++; }
    void recordYellowCard(){ m_yellowCards++; m_careerYellowCards++; }
    void recordRedCard()   { m_redCards++; m_careerRedCards++; }
    void recordCleanSheet(){ m_cleanSheets++; m_careerCleanSheets++; }
    void recordAppearance(){
        m_appearances++;
        m_careerApps++;
    }
    void addMatchRating(double r);
    void applyInjury(int days);
    void recoverDay();
    void applySuspension(int days);
    void reduceSuspension();
    void reduceContract() { if (m_contractLengthMonths > 0) m_contractLengthMonths--; }
    void recoverFitness(int amount) { m_fitness = std::min(100, m_fitness + amount); }
    void drainFitness(int amount)   { m_fitness = std::max(1, m_fitness - amount); }

    // ========== GENERATION ==========
    void generateAttributes(int clubLevel, const std::string& position);
    void generatePlaystyle();
    void generateTraits();
    void generateHiddenAttributes();

private:
    static int clamp(int v, int lo = 1, int hi = 20) {
        return std::max(lo, std::min(hi, v));
    }
};

using PlayerPtr = std::shared_ptr<Player>;

// ========== JSON SERIALISATION ==========
namespace nlohmann {
    template <>
    struct adl_serializer<Player> {
        static void to_json(json& j, const Player& p);
        static void from_json(const json& j, Player& p);
    };
}
// Staff.hpp
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "json.hpp"
using json = nlohmann::json;

// ========== STAFF ROLES ==========
enum class StaffRole {
    Manager,
    AssistantManager,
    GoalkeepingCoach,
    DefensiveCoach,
    AttackingCoach,
    FitnessCoach,
    SetPieceCoach,
    Scout,
    Physio,
    HeadOfYouth
};

// ========== STAFF ATTRIBUTE SET ==========
struct StaffAttributes {
    int tacticalKnowledge = 10;   // Manager/Assistant
    int motivation = 10;          // Manager/Assistant
    int manManagement = 10;       // Manager/Assistant
    int youthDevelopment = 10;    // Manager/Assistant/HeadOfYouth

    int coachingGK = 10;          // GoalkeepingCoach
    int coachingDef = 10;         // DefensiveCoach
    int coachingAtt = 10;         // AttackingCoach
    int coachingFitness = 10;     // FitnessCoach
    int coachingSetPieces = 10;   // SetPieceCoach

    int judgingAbility = 10;      // Scout: current ability estimate accuracy
    int judgingPotential = 10;    // Scout: potential estimate accuracy
    int adaptability = 10;        // Scout: speed of knowledge in new countries

    int physiotherapy = 10;       // Physio: recovery speed
    int prevention = 10;          // Physio: injury prevention
    int diagnosis = 10;           // Physio: injury duration prediction accuracy

    int youthRecruitment = 10;    // HeadOfYouth: intake quality
    int youthCoaching = 10;       // HeadOfYouth: U-18 growth

    int reputation = 20;          // 1-100
};

// ========== STAFF CLASS ==========
class Staff {
private:
    std::string m_name;
    int m_age = 40;
    std::string m_nationality;
    StaffRole m_role;
    StaffAttributes m_attrs;
    int m_contractMonths = 24;
    int m_monthlyWage = 5000;
    bool m_employed = false;
    std::string m_employerTeamName;

    // Relationship with manager (0-100, 50=neutral)
    int m_relationshipWithManager = 50;

public:
    Staff() = default;
    Staff(const std::string& name, StaffRole role);

    // ========== GETTERS ==========
    const std::string& getName() const { return m_name; }
    int getAge() const { return m_age; }
    StaffRole getRole() const { return m_role; }
    const StaffAttributes& getAttributes() const { return m_attrs; }
    StaffAttributes& getAttributes() { return m_attrs; }
    int getContractMonths() const { return m_contractMonths; }
    int getMonthlyWage() const { return m_monthlyWage; }
    bool isEmployed() const { return m_employed; }
    const std::string& getEmployerTeamName() const { return m_employerTeamName; }
    int getRelationshipWithManager() const { return m_relationshipWithManager; }

    // ========== SETTERS ==========
    void setName(const std::string& n) { m_name = n; }
    void setAge(int a) { m_age = a; }
    void setNationality(const std::string& n) { m_nationality = n; }
    void setEmployed(bool e, const std::string& team = "") { m_employed = e; m_employerTeamName = team; }
    void setContract(int months, int wage) { m_contractMonths = months; m_monthlyWage = wage; }

    // ========== RELATIONSHIP ==========
    void improveRelationship(int amount) { m_relationshipWithManager = std::min(100, m_relationshipWithManager + amount); }
    void worsenRelationship(int amount) { m_relationshipWithManager = std::max(0, m_relationshipWithManager - amount); }
    double getEffectivenessMultiplier() const {
        // ±5% depending on relationship
        if (m_relationshipWithManager >= 70) return 1.05;
        if (m_relationshipWithManager <= 30) return 0.95;
        return 1.0;
    }

    // ========== ROLE‑SPECIFIC EFFECTS (called externally) ==========
    double getTrainingQualityMultiplier() const;
    double getInjuryRecoveryMultiplier() const;
    int getYouthIntakeBoost() const;
    double getScoutingAccuracyMultiplier() const;

    // ========== SERIALISATION ==========
    json toJson() const;
    void fromJson(const json& j);
};

using StaffPtr = std::shared_ptr<Staff>;
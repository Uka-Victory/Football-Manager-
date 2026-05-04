// Staff.cpp
#include "Staff.hpp"
#include "Utils.hpp"
#include <algorithm>

Staff::Staff(const std::string& name, StaffRole role) : m_name(name), m_role(role) {
    // Randomise attributes based on role
    auto randomise = [](int base) { return std::max(1, std::min(20, base + Utils::randInt(-3, 3))); };

    switch (role) {
        case StaffRole::Manager:
            m_attrs.tacticalKnowledge = randomise(12);
            m_attrs.motivation = randomise(12);
            m_attrs.manManagement = randomise(12);
            m_attrs.youthDevelopment = randomise(10);
            m_attrs.reputation = Utils::randInt(10, 50);
            break;
        case StaffRole::AssistantManager:
            m_attrs.tacticalKnowledge = randomise(10);
            m_attrs.motivation = randomise(10);
            m_attrs.manManagement = randomise(10);
            m_attrs.youthDevelopment = randomise(10);
            m_attrs.reputation = Utils::randInt(10, 40);
            break;
        case StaffRole::GoalkeepingCoach:
            m_attrs.coachingGK = randomise(13);
            break;
        case StaffRole::DefensiveCoach:
            m_attrs.coachingDef = randomise(13);
            break;
        case StaffRole::AttackingCoach:
            m_attrs.coachingAtt = randomise(13);
            break;
        case StaffRole::FitnessCoach:
            m_attrs.coachingFitness = randomise(13);
            break;
        case StaffRole::SetPieceCoach:
            m_attrs.coachingSetPieces = randomise(13);
            break;
        case StaffRole::Scout:
            m_attrs.judgingAbility = randomise(12);
            m_attrs.judgingPotential = randomise(12);
            m_attrs.adaptability = randomise(10);
            break;
        case StaffRole::Physio:
            m_attrs.physiotherapy = randomise(12);
            m_attrs.prevention = randomise(12);
            m_attrs.diagnosis = randomise(10);
            break;
        case StaffRole::HeadOfYouth:
            m_attrs.youthRecruitment = randomise(12);
            m_attrs.youthCoaching = randomise(12);
            m_attrs.youthDevelopment = randomise(12);
            break;
    }
}

double Staff::getTrainingQualityMultiplier() const {
    double base = 1.0;
    switch (m_role) {
        case StaffRole::Manager:          base += (m_attrs.tacticalKnowledge - 10) * 0.01; break;
        case StaffRole::AttackingCoach:   base += (m_attrs.coachingAtt - 10) * 0.02; break;
        case StaffRole::DefensiveCoach:   base += (m_attrs.coachingDef - 10) * 0.02; break;
        case StaffRole::FitnessCoach:     base += (m_attrs.coachingFitness - 10) * 0.02; break;
        case StaffRole::GoalkeepingCoach: base += (m_attrs.coachingGK - 10) * 0.02; break;
        case StaffRole::SetPieceCoach:    base += (m_attrs.coachingSetPieces - 10) * 0.02; break;
        default: break;
    }
    return base * getEffectivenessMultiplier();
}

double Staff::getInjuryRecoveryMultiplier() const {
    if (m_role == StaffRole::Physio) {
        double base = 1.0 - (m_attrs.physiotherapy - 10) * 0.03;
        base -= (m_attrs.prevention - 10) * 0.02;
        return std::max(0.3, base) * getEffectivenessMultiplier();
    }
    return 1.0;
}

int Staff::getYouthIntakeBoost() const {
    if (m_role == StaffRole::HeadOfYouth) {
        return (m_attrs.youthRecruitment - 10) * 2;
    }
    return 0;
}

double Staff::getScoutingAccuracyMultiplier() const {
    if (m_role == StaffRole::Scout) {
        return (m_attrs.judgingAbility / 20.0) * getEffectivenessMultiplier();
    }
    return 1.0;
}

// ========== JSON ==========
json Staff::toJson() const {
    json j;
    j["name"] = m_name;
    j["age"] = m_age;
    j["nationality"] = m_nationality;
    j["role"] = static_cast<int>(m_role);
    j["attrs"] = {
        {"tacticalKnowledge", m_attrs.tacticalKnowledge},
        {"motivation", m_attrs.motivation},
        {"manManagement", m_attrs.manManagement},
        {"youthDevelopment", m_attrs.youthDevelopment},
        {"coachingGK", m_attrs.coachingGK},
        {"coachingDef", m_attrs.coachingDef},
        {"coachingAtt", m_attrs.coachingAtt},
        {"coachingFitness", m_attrs.coachingFitness},
        {"coachingSetPieces", m_attrs.coachingSetPieces},
        {"judgingAbility", m_attrs.judgingAbility},
        {"judgingPotential", m_attrs.judgingPotential},
        {"adaptability", m_attrs.adaptability},
        {"physiotherapy", m_attrs.physiotherapy},
        {"prevention", m_attrs.prevention},
        {"diagnosis", m_attrs.diagnosis},
        {"youthRecruitment", m_attrs.youthRecruitment},
        {"youthCoaching", m_attrs.youthCoaching},
        {"reputation", m_attrs.reputation}
    };
    j["contractMonths"] = m_contractMonths;
    j["monthlyWage"] = m_monthlyWage;
    j["employed"] = m_employed;
    j["employerTeamName"] = m_employerTeamName;
    j["relationshipWithManager"] = m_relationshipWithManager;
    return j;
}

void Staff::fromJson(const json& j) {
    m_name = j.value("name", "");
    m_age = j.value("age", 40);
    m_nationality = j.value("nationality", "");
    m_role = static_cast<StaffRole>(j.value("role", 0));
    if (j.contains("attrs")) {
        auto& a = j["attrs"];
        m_attrs.tacticalKnowledge = a.value("tacticalKnowledge", 10);
        m_attrs.motivation = a.value("motivation", 10);
        m_attrs.manManagement = a.value("manManagement", 10);
        m_attrs.youthDevelopment = a.value("youthDevelopment", 10);
        m_attrs.coachingGK = a.value("coachingGK", 10);
        m_attrs.coachingDef = a.value("coachingDef", 10);
        m_attrs.coachingAtt = a.value("coachingAtt", 10);
        m_attrs.coachingFitness = a.value("coachingFitness", 10);
        m_attrs.coachingSetPieces = a.value("coachingSetPieces", 10);
        m_attrs.judgingAbility = a.value("judgingAbility", 10);
        m_attrs.judgingPotential = a.value("judgingPotential", 10);
        m_attrs.adaptability = a.value("adaptability", 10);
        m_attrs.physiotherapy = a.value("physiotherapy", 10);
        m_attrs.prevention = a.value("prevention", 10);
        m_attrs.diagnosis = a.value("diagnosis", 10);
        m_attrs.youthRecruitment = a.value("youthRecruitment", 10);
        m_attrs.youthCoaching = a.value("youthCoaching", 10);
        m_attrs.reputation = a.value("reputation", 20);
    }
    m_contractMonths = j.value("contractMonths", 24);
    m_monthlyWage = j.value("monthlyWage", 5000);
    m_employed = j.value("employed", false);
    m_employerTeamName = j.value("employerTeamName", "");
    m_relationshipWithManager = j.value("relationshipWithManager", 50);
}
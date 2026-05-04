// ScoutingEngine.cpp
#include "ScoutingEngine.hpp"
#include "Utils.hpp"
#include <algorithm>

// ========== ASSIGNMENTS ==========
void ScoutingEngine::addAssignment(const ScoutAssignment& assignment) {
    m_assignments.push_back(assignment);
}

void ScoutingEngine::removeAssignment(size_t index) {
    if (index < m_assignments.size())
        m_assignments.erase(m_assignments.begin() + index);
}

const std::vector<ScoutAssignment>& ScoutingEngine::getAssignments() const {
    return m_assignments;
}

// ========== KNOWLEDGE ==========
void ScoutingEngine::increaseKnowledge(const std::string& country, int amount) {
    m_countryKnowledge[country] = std::min(100, m_countryKnowledge[country] + amount);
}

int ScoutingEngine::getKnowledge(const std::string& country) const {
    auto it = m_countryKnowledge.find(country);
    return (it != m_countryKnowledge.end()) ? it->second : 0;
}

void ScoutingEngine::applyPassiveKnowledgeGain() {
    // Each active assignment adds a tiny amount of knowledge
    for (const auto& a : m_assignments) {
        if (!a.active) continue;
        if (!a.targetCountry.empty())
            increaseKnowledge(a.targetCountry, 1);
        // if a competition/club is specified, we could add smaller amounts
    }
}

// ========== SCOUT REPORT ==========
ScoutingEngine::ScoutReportCard ScoutingEngine::generateScoutReport(
    const PlayerPtr& player,
    int scoutJudgingAbility,
    int scoutJudgingPotential,
    int scoutAdaptability) {

    ScoutReportCard card;
    if (!player) return card;

    card.playerId = player->getUniqueId();
    card.playerName = player->getName();

    // Accuracy: base 40% + scout attributes, capped at 95%
    int baseAccuracy = 40 + (scoutJudgingAbility * 2) + (scoutAdaptability / 2);
    int accuracy = std::min(95, baseAccuracy);
    card.scoutAccuracy = accuracy;

    // Estimate overall and potential (range based on accuracy)
    int realOverall = player->getOverall();
    int realPotential = player->getPotentialCeiling();
    int errorMargin = (100 - accuracy) / 5;  // ±0‑4 points error for high accuracy
    card.estimatedOverall = realOverall + Utils::randInt(-errorMargin, errorMargin);
    card.estimatedPotential = realPotential + Utils::randInt(-errorMargin, errorMargin);
    card.estimatedOverall = std::max(1, std::min(20, card.estimatedOverall));
    card.estimatedPotential = std::max(1, std::min(20, card.estimatedPotential));

    // Playstyle (usually visible immediately)
    card.estimatedPlaystyle = std::to_string(static_cast<int>(player->getPlaystyle()));

    // Attribute ranges: wider range for lower accuracy
    auto attrError = [&](int realVal) -> std::pair<int,int> {
        int halfRange = (100 - accuracy) / 10;
        int lo = std::max(1, realVal - Utils::randInt(0, halfRange));
        int hi = std::min(20, realVal + Utils::randInt(0, halfRange));
        return {lo, hi};
    };
    card.attributeRanges["Shooting"]     = attrError(player->getShooting());
    card.attributeRanges["Passing"]      = attrError(player->getPassing());
    card.attributeRanges["Dribbling"]    = attrError(player->getDribbling());
    card.attributeRanges["Tackling"]     = attrError(player->getTackling());
    card.attributeRanges["Pace"]         = attrError(player->getPace());
    card.attributeRanges["Positioning"]  = attrError(player->getPositioning());
    card.attributeRanges["Composure"]    = attrError(player->getComposure());
    card.attributeRanges["Vision"]       = attrError(player->getVision());
    card.attributeRanges["Decisions"]    = attrError(player->getDecisions());

    // Value and contract (approximate)
    card.estimatedValue = player->getOverall() * 500'000 + Utils::randInt(-100'000, 100'000);
    card.estimatedWage = player->getWeeklyWage() + Utils::randInt(-200, 200);
    card.contractMonthsLeft = player->getContractLengthMonths();

    // Verdict (simple)
    if (card.estimatedOverall >= 15 && card.estimatedPotential >= 17)
        card.verdict = "Strongly Recommend";
    else if (card.estimatedOverall >= 11)
        card.verdict = "Recommend";
    else if (card.estimatedOverall >= 7)
        card.verdict = "Monitor";
    else
        card.verdict = "Not Recommended";

    return card;
}

// ========== TALENT DISCOVERY ==========
std::vector<PlayerPtr> ScoutingEngine::discoverTalent(
    const std::string& country,
    const std::map<std::string, PlayerPtr>& globalPlayers,
    int scoutJudgingPotential) {

    std::vector<PlayerPtr> discovered;
    int chancePerPlayer = 1 + scoutJudgingPotential / 4;  // 1‑6% per eligible player

    for (const auto& kv : globalPlayers) {
        const auto& p = kv.second;
        if (!p) continue;
        // Only discover youth (16‑21) in the target country
        if (p->getAge() > 21 || p->getAge() < 16) continue;
        if (p->getNationality() != country) continue;

        if (Utils::randInt(1, 100) <= chancePerPlayer) {
            discovered.push_back(p);
        }
    }
    return discovered;
}

// ========== JSON ==========
json ScoutingEngine::toJson() const {
    json j;
    j["countryKnowledge"] = m_countryKnowledge;
    j["assignments"] = json::array();
    for (const auto& a : m_assignments) {
        j["assignments"].push_back({
            {"targetCountry", a.targetCountry},
            {"targetCompetition", a.targetCompetition},
            {"targetClub", a.targetClub},
            {"weeksAssigned", a.weeksAssigned},
            {"active", a.active}
        });
    }
    return j;
}

void ScoutingEngine::fromJson(const json& j) {
    if (j.contains("countryKnowledge"))
        m_countryKnowledge = j["countryKnowledge"].get<std::map<std::string,int>>();
    m_assignments.clear();
    if (j.contains("assignments") && j["assignments"].is_array()) {
        for (const auto& aj : j["assignments"]) {
            ScoutAssignment a;
            a.targetCountry = aj.value("targetCountry", "");
            a.targetCompetition = aj.value("targetCompetition", "");
            a.targetClub = aj.value("targetClub", "");
            a.weeksAssigned = aj.value("weeksAssigned", 0);
            a.active = aj.value("active", true);
            m_assignments.push_back(a);
        }
    }
}
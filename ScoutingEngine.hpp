// ScoutingEngine.hpp
#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Player.hpp"
#include "Team.hpp"
#include "json.hpp"
using json = nlohmann::json;

struct ScoutAssignment {
    std::string targetCountry;       // country or region
    std::string targetCompetition;   // or specific competition
    std::string targetClub;          // or specific club (youth raid)
    int weeksAssigned = 0;
    bool active = true;
};

class ScoutingEngine {
private:
    // Knowledge per country (0‑100)
    std::map<std::string, int> m_countryKnowledge;
    std::vector<ScoutAssignment> m_assignments;
    std::string m_clubName;

public:
    ScoutingEngine() = default;
    void setClubName(const std::string& name) { m_clubName = name; }

    // Assignments
    void addAssignment(const ScoutAssignment& assignment);
    void removeAssignment(size_t index);
    const std::vector<ScoutAssignment>& getAssignments() const;

    // Knowledge
    void increaseKnowledge(const std::string& country, int amount);
    int getKnowledge(const std::string& country) const;
    void applyPassiveKnowledgeGain();   // daily/weekly tick

    // Player scouting (generates a report card with knowledge‑based accuracy)
    struct ScoutReportCard {
        std::string playerId;
        std::string playerName;
        int estimatedOverall = 0;
        int estimatedPotential = 0;
        std::string estimatedPlaystyle;
        std::map<std::string, std::pair<int,int>> attributeRanges; // min, max
        int scoutAccuracy = 50;  // 0‑100%
        int estimatedValue = 0;
        int estimatedWage = 0;
        int contractMonthsLeft = 0;
        std::string verdict;    // "Strongly Recommend" etc.
    };
    ScoutReportCard generateScoutReport(const PlayerPtr& player,
                                        int scoutJudgingAbility,
                                        int scoutJudgingPotential,
                                        int scoutAdaptability);

    // Talent discovery (regens)
    std::vector<PlayerPtr> discoverTalent(const std::string& country,
                                          const std::map<std::string, PlayerPtr>& globalPlayers,
                                          int scoutJudgingPotential);

    // Serialisation
    json toJson() const;
    void fromJson(const json& j);
};
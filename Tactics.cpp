// Tactics.cpp
#include "Tactics.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <unordered_set>

// ========== CONSTRUCTOR ==========
TeamTactics::TeamTactics() {
    setFormation("4-3-3");  // default
}

// ========== PREDEFINED FORMATIONS ==========
std::vector<Formation> TeamTactics::getPredefinedFormations() {
    std::vector<Formation> formations;

    // Helper to create a slot
    auto slot = [](const std::string& pos, const std::vector<Playstyle>& roles,
                   Playstyle defRole) -> FormationSlot {
        FormationSlot s;
        s.positionGroup = pos;
        s.allowedRoles = roles;
        s.defaultRole = defRole;
        return s;
    };

    // GK allowed roles
    std::vector<Playstyle> gkRoles = {Playstyle::SweeperKeeper, Playstyle::TraditionalGoalkeeper};
    // CB allowed roles
    std::vector<Playstyle> cbRoles = {Playstyle::BallPlayingDefender, Playstyle::Sweeper,
                                       Playstyle::Stopper, Playstyle::AnchorMan};
    // LB/RB allowed roles
    std::vector<Playstyle> fbRoles = {Playstyle::DefensiveFullBack, Playstyle::AttackingFullBack,
                                       Playstyle::InvertedWingBack};
    // DM allowed roles
    std::vector<Playstyle> dmRoles = {Playstyle::AnchorMan, Playstyle::DeepLyingPlaymaker,
                                       Playstyle::BallPlayingDefender, Playstyle::BoxToBox};
    // CM allowed roles
    std::vector<Playstyle> cmRoles = {Playstyle::DeepLyingPlaymaker, Playstyle::BoxToBox,
                                       Playstyle::AdvancedPlaymaker, Playstyle::AnchorMan};
    // LM/RM allowed roles
    std::vector<Playstyle> wmRoles = {Playstyle::Winger, Playstyle::InsideForward,
                                       Playstyle::Raumdeuter, Playstyle::AdvancedPlaymaker};
    // AM allowed roles
    std::vector<Playstyle> amRoles = {Playstyle::AdvancedPlaymaker, Playstyle::False9,
                                       Playstyle::InsideForward, Playstyle::Raumdeuter,
                                       Playstyle::DeepLyingForward};
    // LW/RW allowed roles
    std::vector<Playstyle> wingRoles = {Playstyle::Winger, Playstyle::InsideForward,
                                         Playstyle::Raumdeuter, Playstyle::False9};
    // ST allowed roles
    std::vector<Playstyle> stRoles = {Playstyle::TargetMan, Playstyle::Poacher,
                                       Playstyle::False9, Playstyle::DeepLyingForward,
                                       Playstyle::InsideForward};

    // ----- 4-4-2 -----
    {
        Formation f;
        f.name = "4-4-2";
        f.slots = {
            slot("GK", gkRoles, Playstyle::TraditionalGoalkeeper),
            slot("LB", fbRoles, Playstyle::DefensiveFullBack),
            slot("CB", cbRoles, Playstyle::Stopper),
            slot("CB", cbRoles, Playstyle::BallPlayingDefender),
            slot("RB", fbRoles, Playstyle::DefensiveFullBack),
            slot("LM", wmRoles, Playstyle::Winger),
            slot("CM", cmRoles, Playstyle::BoxToBox),
            slot("CM", cmRoles, Playstyle::DeepLyingPlaymaker),
            slot("RM", wmRoles, Playstyle::Winger),
            slot("ST", stRoles, Playstyle::TargetMan),
            slot("ST", stRoles, Playstyle::Poacher)
        };
        formations.push_back(f);
    }

    // ----- 4-3-3 -----
    {
        Formation f;
        f.name = "4-3-3";
        f.slots = {
            slot("GK", gkRoles, Playstyle::TraditionalGoalkeeper),
            slot("LB", fbRoles, Playstyle::AttackingFullBack),
            slot("CB", cbRoles, Playstyle::BallPlayingDefender),
            slot("CB", cbRoles, Playstyle::Stopper),
            slot("RB", fbRoles, Playstyle::AttackingFullBack),
            slot("CM", cmRoles, Playstyle::DeepLyingPlaymaker),
            slot("CM", cmRoles, Playstyle::BoxToBox),
            slot("CM", cmRoles, Playstyle::AdvancedPlaymaker),
            slot("LW", wingRoles, Playstyle::InsideForward),
            slot("ST", stRoles, Playstyle::Poacher),
            slot("RW", wingRoles, Playstyle::Winger)
        };
        formations.push_back(f);
    }

    // ----- 4-2-3-1 -----
    {
        Formation f;
        f.name = "4-2-3-1";
        f.slots = {
            slot("GK", gkRoles, Playstyle::TraditionalGoalkeeper),
            slot("LB", fbRoles, Playstyle::DefensiveFullBack),
            slot("CB", cbRoles, Playstyle::Stopper),
            slot("CB", cbRoles, Playstyle::BallPlayingDefender),
            slot("RB", fbRoles, Playstyle::DefensiveFullBack),
            slot("DM", dmRoles, Playstyle::AnchorMan),
            slot("DM", dmRoles, Playstyle::DeepLyingPlaymaker),
            slot("LW", wingRoles, Playstyle::InsideForward),
            slot("AM", amRoles, Playstyle::AdvancedPlaymaker),
            slot("RW", wingRoles, Playstyle::Winger),
            slot("ST", stRoles, Playstyle::Poacher)
        };
        formations.push_back(f);
    }

    // ----- 3-5-2 -----
    {
        Formation f;
        f.name = "3-5-2";
        f.slots = {
            slot("GK", gkRoles, Playstyle::TraditionalGoalkeeper),
            slot("CB", cbRoles, Playstyle::Sweeper),
            slot("CB", cbRoles, Playstyle::Stopper),
            slot("CB", cbRoles, Playstyle::BallPlayingDefender),
            slot("LM", wmRoles, Playstyle::Winger),
            slot("CM", cmRoles, Playstyle::BoxToBox),
            slot("CM", cmRoles, Playstyle::DeepLyingPlaymaker),
            slot("CM", cmRoles, Playstyle::AdvancedPlaymaker),
            slot("RM", wmRoles, Playstyle::Winger),
            slot("ST", stRoles, Playstyle::TargetMan),
            slot("ST", stRoles, Playstyle::Poacher)
        };
        formations.push_back(f);
    }

    // ----- 3-4-3 -----
    {
        Formation f;
        f.name = "3-4-3";
        f.slots = {
            slot("GK", gkRoles, Playstyle::TraditionalGoalkeeper),
            slot("CB", cbRoles, Playstyle::Stopper),
            slot("CB", cbRoles, Playstyle::Sweeper),
            slot("CB", cbRoles, Playstyle::BallPlayingDefender),
            slot("LM", wmRoles, Playstyle::Winger),
            slot("CM", cmRoles, Playstyle::BoxToBox),
            slot("CM", cmRoles, Playstyle::DeepLyingPlaymaker),
            slot("RM", wmRoles, Playstyle::Winger),
            slot("LW", wingRoles, Playstyle::InsideForward),
            slot("ST", stRoles, Playstyle::TargetMan),
            slot("RW", wingRoles, Playstyle::InsideForward)
        };
        formations.push_back(f);
    }

    // ----- 5-3-2 -----
    {
        Formation f;
        f.name = "5-3-2";
        f.slots = {
            slot("GK", gkRoles, Playstyle::TraditionalGoalkeeper),
            slot("LB", fbRoles, Playstyle::DefensiveFullBack),
            slot("CB", cbRoles, Playstyle::Stopper),
            slot("CB", cbRoles, Playstyle::Sweeper),
            slot("CB", cbRoles, Playstyle::BallPlayingDefender),
            slot("RB", fbRoles, Playstyle::DefensiveFullBack),
            slot("CM", cmRoles, Playstyle::BoxToBox),
            slot("CM", cmRoles, Playstyle::DeepLyingPlaymaker),
            slot("CM", cmRoles, Playstyle::AdvancedPlaymaker),
            slot("ST", stRoles, Playstyle::TargetMan),
            slot("ST", stRoles, Playstyle::Poacher)
        };
        formations.push_back(f);
    }

    return formations;
}

// ========== SET FORMATION ==========
void TeamTactics::setFormation(const std::string& formationName) {
    auto formations = getPredefinedFormations();
    for (const auto& f : formations) {
        if (f.name == formationName) {
            m_formation = f;
            m_roleAssignments.clear();
            for (const auto& slot : m_formation.slots)
                m_roleAssignments[slot.positionGroup] = slot.defaultRole;
            return;
        }
    }
    // If not found, default to 4-3-3
    setFormation("4-3-3");
}

// ========== ROLE ASSIGNMENT ==========
void TeamTactics::assignRoleToSlot(const std::string& positionGroup, Playstyle role) {
    m_roleAssignments[positionGroup] = role;
}

Playstyle TeamTactics::getRoleForSlot(const std::string& positionGroup) const {
    auto it = m_roleAssignments.find(positionGroup);
    if (it != m_roleAssignments.end()) return it->second;
    // Return default
    for (const auto& slot : m_formation.slots)
        if (slot.positionGroup == positionGroup) return slot.defaultRole;
    return Playstyle::BoxToBox; // fallback
}

// ========== ROLE SUITABILITY ==========
double TeamTactics::calculateRoleSuitability(Playstyle natural, Playstyle assigned, int versatility) {
    if (natural == assigned) return 1.0; // perfect match

    // Define similarity groups (subjective but reasonable)
    auto group = [](Playstyle p) -> int {
        switch (p) {
            case Playstyle::Poacher:
            case Playstyle::InsideForward:
            case Playstyle::Raumdeuter:
            case Playstyle::TargetMan:
            case Playstyle::False9:
            case Playstyle::DeepLyingForward:
                return 1; // attackers
            case Playstyle::AdvancedPlaymaker:
            case Playstyle::DeepLyingPlaymaker:
            case Playstyle::BoxToBox:
                return 2; // midfield creators
            case Playstyle::AnchorMan:
            case Playstyle::BallPlayingDefender:
            case Playstyle::Sweeper:
            case Playstyle::Stopper:
            case Playstyle::DefensiveFullBack:
            case Playstyle::AttackingFullBack:
            case Playstyle::InvertedWingBack:
                return 3; // defensive
            case Playstyle::SweeperKeeper:
            case Playstyle::TraditionalGoalkeeper:
                return 4; // goalkeepers
            default: return 5;
        }
    };

    int natGrp = group(natural);
    int assGrp = group(assigned);

    double base = 0.6; // different group penalty
    if (natGrp == assGrp) base = 0.9; // same broad group

    // Versatility reduces penalty
    double penalty = 1.0 - base;
    double reduction = versatility * 0.02; // up to 0.4 reduction
    double finalSuitability = base + penalty * reduction;
    return std::min(1.0, std::max(0.5, finalSuitability));
}

// ========== SELECT BEST XI ==========
std::vector<PlayerPtr> TeamTactics::selectBestXI(
    const Formation& formation,
    const std::vector<PlayerPtr>& availablePlayers,
    const std::map<std::string, Playstyle>& roleAssignments) {

    std::vector<PlayerPtr> xi;
    std::unordered_set<std::string> usedPlayerIds;

    for (const auto& slot : formation.slots) {
        const std::string& pos = slot.positionGroup;
        Playstyle assignedRole = Playstyle::BoxToBox; // default
        auto it = roleAssignments.find(pos);
        if (it != roleAssignments.end()) assignedRole = it->second;

        PlayerPtr bestPlayer = nullptr;
        double bestScore = -1.0;

        for (const auto& player : availablePlayers) {
            if (!player->isAvailable() || player->getFitness() < 60) continue;
            if (usedPlayerIds.count(player->getUniqueId())) continue;

            // Position match check
            bool positionMatch = (player->getPrimaryPosition() == pos ||
                                  player->getSecondaryPosition() == pos);
            if (!positionMatch) continue;

            // Role suitability
            double roleSuitability = calculateRoleSuitability(
                player->getPlaystyle(), assignedRole, player->getVersatility());

            // Overall ability (weighted)
            double ability = player->getOverall() / 20.0; // scale to 0-1

            // Form (last 5 rating average scaled)
            double form = 0.5;
            const auto& ratings = player->getLast5Ratings();
            if (!ratings.empty()) {
                double avg = 0.0;
                for (double r : ratings) avg += r;
                avg /= ratings.size();
                form = avg / 10.0; // scale to 0-1
            }

            // Fitness factor
            double fitnessFactor = player->getFitness() / 100.0;

            // Composite score
            double score = ability * 0.4 + roleSuitability * 0.25 + form * 0.2 + fitnessFactor * 0.15;
            if (score > bestScore) {
                bestScore = score;
                bestPlayer = player;
            }
        }

        if (bestPlayer) {
            xi.push_back(bestPlayer);
            usedPlayerIds.insert(bestPlayer->getUniqueId());
        }
    }

    return xi;
}

// ========== JSON ==========
json TeamTactics::toJson() const {
    json j;
    j["formationName"] = m_formation.name;
    j["roles"] = json::object();
    for (const auto& kv : m_roleAssignments) {
        j["roles"][kv.first] = static_cast<int>(kv.second);
    }
    j["instructions"] = {
        {"mentality", static_cast<int>(m_instructions.mentality)},
        {"passingStyle", static_cast<int>(m_instructions.passingStyle)},
        {"tempo", static_cast<int>(m_instructions.tempo)},
        {"pressingIntensity", m_instructions.pressingIntensity},
        {"defensiveLine", static_cast<int>(m_instructions.defensiveLine)},
        {"width", static_cast<int>(m_instructions.width)},
        {"creativeFreedom", static_cast<int>(m_instructions.creativeFreedom)}
    };
    j["setPieces"] = {
        {"cornerTakers", m_setPieces.cornerTakers},
        {"cornerDelivery", static_cast<int>(m_setPieces.cornerDelivery)},
        {"cornerMarking", static_cast<int>(m_setPieces.cornerMarking)},
        {"stayForwardOnDefensiveCorner", m_setPieces.stayForwardOnDefensiveCorner},
        {"freeKickTakers", m_setPieces.freeKickTakers},
        {"freeKickDelivery", static_cast<int>(m_setPieces.freeKickDelivery)},
        {"wallSize", m_setPieces.wallSize},
        {"penaltyTakers", m_setPieces.penaltyTakers}
    };
    return j;
}

void TeamTactics::fromJson(const json& j) {
    std::string formationName = j.value("formationName", "4-3-3");
    setFormation(formationName);
    m_roleAssignments.clear();
    if (j.contains("roles") && j["roles"].is_object()) {
        for (auto it = j["roles"].begin(); it != j["roles"].end(); ++it) {
            Playstyle role = static_cast<Playstyle>(it.value().get<int>());
            m_roleAssignments[it.key()] = role;
        }
    }
    if (j.contains("instructions")) {
        auto& i = j["instructions"];
        m_instructions.mentality = static_cast<Mentality>(i.value("mentality", 2));
        m_instructions.passingStyle = static_cast<PassingStyle>(i.value("passingStyle", 1));
        m_instructions.tempo = static_cast<Tempo>(i.value("tempo", 1));
        m_instructions.pressingIntensity = i.value("pressingIntensity", 30);
        m_instructions.defensiveLine = static_cast<DefensiveLine>(i.value("defensiveLine", 1));
        m_instructions.width = static_cast<Width>(i.value("width", 1));
        m_instructions.creativeFreedom = static_cast<CreativeFreedom>(i.value("creativeFreedom", 0));
    }
    if (j.contains("setPieces")) {
        auto& s = j["setPieces"];
        m_setPieces.cornerTakers = s.value("cornerTakers", std::vector<std::string>{});
        m_setPieces.cornerDelivery = static_cast<CornerDelivery>(s.value("cornerDelivery", 1));
        m_setPieces.cornerMarking = static_cast<MarkingType>(s.value("cornerMarking", 1));
        m_setPieces.stayForwardOnDefensiveCorner = s.value("stayForwardOnDefensiveCorner", true);
        m_setPieces.freeKickTakers = s.value("freeKickTakers", std::vector<std::string>{});
        m_setPieces.freeKickDelivery = static_cast<FreeKickDelivery>(s.value("freeKickDelivery", 1));
        m_setPieces.wallSize = s.value("wallSize", 4);
        m_setPieces.penaltyTakers = s.value("penaltyTakers", std::vector<std::string>{});
    }
}
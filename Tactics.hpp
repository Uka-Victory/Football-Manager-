// Tactics.hpp
#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Player.hpp"
#include "json.hpp"
using json = nlohmann::json;

// ========== FORMATION SLOT ==========
struct FormationSlot {
    std::string positionGroup;          // "GK","CB","LB","RB","DM","CM","LM","RM","AM","LW","RW","ST"
    std::vector<Playstyle> allowedRoles; // suitable playstyles for this slot
    Playstyle defaultRole;              // the role the AI / auto‑select uses
};

// ========== FORMATION ==========
struct Formation {
    std::string name;                   // "4‑3‑3", "4‑4‑2", etc.
    std::vector<FormationSlot> slots;   // exactly 11 slots
};

// ========== TEAM INSTRUCTIONS ==========
enum class Mentality { VeryDefensive, Defensive, Balanced, Attacking, VeryAttacking };
enum class PassingStyle { Short, Mixed, Direct };
enum class Tempo { Slow, Normal, Fast };
enum class DefensiveLine { Deep, Normal, High };
enum class Width { Narrow, Normal, Wide };
enum class CreativeFreedom { Disciplined, Expressive };

struct TeamInstructions {
    Mentality mentality = Mentality::Balanced;
    PassingStyle passingStyle = PassingStyle::Mixed;
    Tempo tempo = Tempo::Normal;
    int pressingIntensity = 30;        // 0‑100
    DefensiveLine defensiveLine = DefensiveLine::Normal;
    Width width = Width::Normal;
    CreativeFreedom creativeFreedom = CreativeFreedom::Balanced;
};

// ========== SET‑PIECE ROUTINES ==========
enum class CornerDelivery { Short, NearPost, FarPost, PenaltySpot, EdgeOfBox };
enum class FreeKickDelivery { Direct, CrossNear, CrossFar, ShortPass, EdgeOfBox };
enum class MarkingType { ManToMan, Zonal };

struct SetPieceRoutines {
    // Corner takers (left & right share same list)
    std::vector<std::string> cornerTakers;       // player IDs, ordered 1‑5

    // Attacking corners
    CornerDelivery cornerDelivery = CornerDelivery::NearPost;
    MarkingType cornerMarking = MarkingType::Zonal;  // for defending corners
    bool stayForwardOnDefensiveCorner = true;    // keep 1‑2 players up

    // Free‑kick takers
    std::vector<std::string> freeKickTakers;

    // Attacking free kicks
    FreeKickDelivery freeKickDelivery = FreeKickDelivery::CrossFar;
    int wallSize = 4;

    // Penalty takers
    std::vector<std::string> penaltyTakers;
};

// ========== TEAM TACTICS ==========
class TeamTactics {
private:
    Formation m_formation;
    std::map<std::string, Playstyle> m_roleAssignments;  // slot positionGroup -> assigned playstyle
    TeamInstructions m_instructions;
    SetPieceRoutines m_setPieces;

public:
    TeamTactics();

    // Load a predefined formation by name
    void setFormation(const std::string& formationName);
    const Formation& getFormation() const { return m_formation; }

    // Role assignment
    void assignRoleToSlot(const std::string& positionGroup, Playstyle role);
    Playstyle getRoleForSlot(const std::string& positionGroup) const;

    // Instructions
    void setInstructions(const TeamInstructions& instr) { m_instructions = instr; }
    const TeamInstructions& getInstructions() const { return m_instructions; }

    // Set pieces
    void setSetPieceRoutines(const SetPieceRoutines& routines) { m_setPieces = routines; }
    const SetPieceRoutines& getSetPieceRoutines() const { return m_setPieces; }

    // Helpers
    static std::vector<Formation> getPredefinedFormations();
    static double calculateRoleSuitability(Playstyle natural, Playstyle assigned, int versatility);
    static std::vector<PlayerPtr> selectBestXI(const Formation& formation,
                                               const std::vector<PlayerPtr>& availablePlayers,
                                               const std::map<std::string, Playstyle>& roleAssignments);

    // Serialisation
    json toJson() const;
    void fromJson(const json& j);
};
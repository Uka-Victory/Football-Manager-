#include "Player.hpp"
#include <algorithm>
#include <cmath>

namespace FootballManager {

    void CurrentSeasonStats::reset() {
        appearances = 0; goals = 0; assists = 0; cleanSheets = 0;
        yellowCards = 0; redCards = 0; averageRating = 0.0;
        accumulatedXG = 0.0; keyPasses = 0;
    }

    void CurrentSeasonStats::updateRating(double newMatchRating) {
        if (appearances == 0) {
            averageRating = newMatchRating;
        } else {
            averageRating = ((averageRating * appearances) + newMatchRating) / (appearances + 1);
        }
    }

    Player::Player(std::string playerId, std::string playerName, int playerAge, Position pos)
        : id(playerId), name(playerName), age(playerAge), primaryPosition(pos),
          finishing(10), passing(10), tackling(10), pace(10), vision(10), 
          composure(10), decisions(10), anticipation(10), positioning(10),
          currentReputation(5), homeReputation(5), worldReputation(5),
          weeklyWage(0), contractEndYear(0), academyJoinYear(0),
          status(SquadStatus::EmergencyProspect), manualRank(99), fitness(100) {}

    int Player::getAttribute(const std::string& attrName) const {
        if (attrName == "finishing") return finishing;
        if (attrName == "passing") return passing;
        if (attrName == "tackling") return tackling;
        if (attrName == "pace") return pace;
        if (attrName == "vision") return vision;
        if (attrName == "composure") return composure;
        if (attrName == "decisions") return decisions;
        if (attrName == "anticipation") return anticipation;
        if (attrName == "positioning") return positioning;
        return 1;
    }

    void Player::setAttribute(const std::string& attrName, int newValue) {
        int clampedValue = std::max(1, std::min(20, newValue)); // Force 1-20 Scale
        if (attrName == "finishing") finishing = clampedValue;
        else if (attrName == "passing") passing = clampedValue;
        else if (attrName == "tackling") tackling = clampedValue;
        else if (attrName == "pace") pace = clampedValue;
        else if (attrName == "vision") vision = clampedValue;
        else if (attrName == "composure") composure = clampedValue;
        else if (attrName == "decisions") decisions = clampedValue;
        else if (attrName == "anticipation") anticipation = clampedValue;
        else if (attrName == "positioning") positioning = clampedValue;
    }

    void Player::assignContract(int wage, int endYear) {
        weeklyWage = wage;
        contractEndYear = endYear;
    }

    double Player::calculateRoleScore(Position role) const {
        double score = 0.0;
        switch (role) {
            case Position::ST: score = (finishing * 0.4) + (composure * 0.3) + (anticipation * 0.2) + (pace * 0.1); break;
            case Position::CM: score = (passing * 0.4) + (vision * 0.3) + (decisions * 0.2) + (positioning * 0.1); break;
            case Position::CB: score = (tackling * 0.4) + (positioning * 0.3) + (anticipation * 0.2) + (decisions * 0.1); break;
            default: score = (passing * 0.3) + (tackling * 0.3) + (pace * 0.2) + (decisions * 0.2); break;
        }
        return score;
    }

    double Player::getSelectionIndex() const {
        double roleScore = calculateRoleScore(primaryPosition);
        double fitnessMultiplier = fitness / 100.0;
        double hierarchyWeight = static_cast<double>(status) / 20.0; 
        return (roleScore * fitnessMultiplier) * hierarchyWeight;
    }

    void Player::processMidnightWipe() {
        seasonStats.reset();
    }

} // namespace FootballManager
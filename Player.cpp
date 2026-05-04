// Player.cpp
#include "Player.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <map>

// ========== INTERNAL HELPERS ==========

static int clamp20(int v, int lo = 1, int hi = 20) {
    return std::max(lo, std::min(hi, v));
}

// Position profiles: each attribute gets a weight (0–40) used for generation.
// Higher weight → attribute will be closer to the base value (clubLevel/5 + rand).
struct AttrWeights {
    int shooting = 0, passing = 0, crossing = 0, dribbling = 0, tackling = 0, heading = 0;
    int positioning = 0, composure = 0, vision = 0, workRate = 0, leadership = 0, decisions = 0;
    int pace = 0, acceleration = 0, stamina = 0, strength = 0, agility = 0;
};

static AttrWeights getPositionWeights(const std::string& pos) {
    AttrWeights w;
    if (pos == "GK") {
        w.passing = 8; w.crossing = 2; w.dribbling = 3; w.tackling = 1; w.heading = 3;
        w.positioning = 24; w.composure = 15; w.vision = 6; w.workRate = 8; w.leadership = 6; w.decisions = 10;
        w.pace = 4; w.acceleration = 5; w.stamina = 5; w.strength = 4; w.agility = 10;
    } else if (pos == "CB") {
        w.shooting = 2; w.passing = 6; w.crossing = 2; w.dribbling = 2; w.tackling = 15; w.heading = 14;
        w.positioning = 18; w.composure = 10; w.vision = 4; w.workRate = 8; w.leadership = 5; w.decisions = 8;
        w.pace = 6; w.acceleration = 5; w.stamina = 8; w.strength = 13; w.agility = 3;
    } else if (pos == "LB" || pos == "RB") {
        w.shooting = 2; w.passing = 8; w.crossing = 12; w.dribbling = 10; w.tackling = 12; w.heading = 3;
        w.positioning = 10; w.composure = 5; w.vision = 4; w.workRate = 10; w.leadership = 2; w.decisions = 7;
        w.pace = 13; w.acceleration = 10; w.stamina = 12; w.strength = 5; w.agility = 5;
    } else if (pos == "DM") {
        w.shooting = 3; w.passing = 10; w.crossing = 3; w.dribbling = 5; w.tackling = 14; w.heading = 6;
        w.positioning = 12; w.composure = 8; w.vision = 8; w.workRate = 12; w.leadership = 3; w.decisions = 10;
        w.pace = 4; w.acceleration = 5; w.stamina = 9; w.strength = 7; w.agility = 4;
    } else if (pos == "CM") {
        w.shooting = 6; w.passing = 14; w.crossing = 5; w.dribbling = 9; w.tackling = 7; w.heading = 3;
        w.positioning = 7; w.composure = 8; w.vision = 12; w.workRate = 10; w.leadership = 2; w.decisions = 10;
        w.pace = 6; w.acceleration = 6; w.stamina = 9; w.strength = 4; w.agility = 5;
    } else if (pos == "LM" || pos == "RM") {
        w.shooting = 6; w.passing = 11; w.crossing = 14; w.dribbling = 12; w.tackling = 5; w.heading = 3;
        w.positioning = 6; w.composure = 5; w.vision = 7; w.workRate = 10; w.leadership = 1; w.decisions = 6;
        w.pace = 13; w.acceleration = 10; w.stamina = 12; w.strength = 3; w.agility = 6;
    } else if (pos == "AM") {
        w.shooting = 12; w.passing = 12; w.crossing = 6; w.dribbling = 13; w.tackling = 2; w.heading = 3;
        w.positioning = 7; w.composure = 10; w.vision = 13; w.workRate = 6; w.leadership = 1; w.decisions = 10;
        w.pace = 7; w.acceleration = 8; w.stamina = 7; w.strength = 2; w.agility = 7;
    } else if (pos == "LW" || pos == "RW") {
        w.shooting = 14; w.passing = 8; w.crossing = 12; w.dribbling = 15; w.tackling = 2; w.heading = 3;
        w.positioning = 5; w.composure = 7; w.vision = 7; w.workRate = 7; w.leadership = 1; w.decisions = 6;
        w.pace = 14; w.acceleration = 13; w.stamina = 10; w.strength = 2; w.agility = 9;
    } else if (pos == "ST") {
        w.shooting = 22; w.passing = 5; w.crossing = 3; w.dribbling = 9; w.tackling = 1; w.heading = 10;
        w.positioning = 12; w.composure = 12; w.vision = 5; w.workRate = 5; w.leadership = 1; w.decisions = 6;
        w.pace = 10; w.acceleration = 8; w.stamina = 6; w.strength = 8; w.agility = 3;
    }
    return w;
}

// ========== PLAYER METHODS ==========

void Player::addMatchRating(double r) {
    m_last5Ratings.push_back(r);
    if (m_last5Ratings.size() > 5) m_last5Ratings.erase(m_last5Ratings.begin());
    double avgForm = 0.0;
    for (double v : m_last5Ratings) avgForm += v;
    avgForm /= m_last5Ratings.size();
    m_form = static_cast<int>(avgForm * 10.0);
    if (m_appearances > 0) {
        double total = m_avgRating * (m_appearances - 1);
        m_avgRating = (total + r) / m_appearances;
    } else {
        m_avgRating = r;
    }
}

void Player::applyInjury(int days) {
    if (m_injured) return;
    m_injured = true;
    m_injuryDaysRemaining = days;
}

void Player::recoverDay() {
    if (m_injured && m_injuryDaysRemaining > 0) {
        m_injuryDaysRemaining--;
        if (m_injuryDaysRemaining == 0) m_injured = false;
    }
}

void Player::applySuspension(int days) {
    m_suspended = true;
    m_suspensionDaysRemaining = days;
}

void Player::reduceSuspension() {
    if (m_suspended && m_suspensionDaysRemaining > 0) {
        m_suspensionDaysRemaining--;
        if (m_suspensionDaysRemaining == 0) m_suspended = false;
    }
}

// ========== GENERATION ==========

void Player::generateAttributes(int clubLevel, const std::string& position) {
    // Base attribute value from club level (1–20)
    int base = clubLevel;  // clubLevel is 1–20
    base = clamp20(base, 5, 20);  // floor at 5

    auto w = getPositionWeights(position);

    auto gen = [&](int weight) -> int {
        if (weight == 0) return clamp20(Utils::randInt(1, base - 3));
        // Scale: weight / 10.0 * base gives a target; add random jitter
        double target = base * (weight / 10.0);
        int val = static_cast<int>(target + Utils::randInt(-3, 3));
        return clamp20(val);
    };

    m_shooting     = gen(w.shooting);
    m_passing      = gen(w.passing);
    m_crossing     = gen(w.crossing);
    m_dribbling    = gen(w.dribbling);
    m_tackling     = gen(w.tackling);
    m_heading      = gen(w.heading);
    m_positioning  = gen(w.positioning);
    m_composure    = gen(w.composure);
    m_vision       = gen(w.vision);
    m_workRate     = gen(w.workRate);
    m_leadership   = gen(w.leadership);
    m_decisions    = gen(w.decisions);
    m_pace         = gen(w.pace);
    m_acceleration = gen(w.acceleration);
    m_stamina      = gen(w.stamina);
    m_strength     = gen(w.strength);
    m_agility      = gen(w.agility);

    m_potentialCeiling = clamp20(getOverall() + Utils::randInt(1, 6));
}

void Player::generatePlaystyle() {
    std::map<Playstyle, int> weights;
    std::string pos = m_primaryPosition;

    if (pos == "GK") {
        weights[Playstyle::SweeperKeeper]          = 25;
        weights[Playstyle::TraditionalGoalkeeper]  = 75;
    } else if (pos == "CB") {
        weights[Playstyle::BallPlayingDefender]    = 25;
        weights[Playstyle::Sweeper]                = 20;
        weights[Playstyle::Stopper]                = 35;
        weights[Playstyle::AnchorMan]              = 20;
    } else if (pos == "LB" || pos == "RB") {
        weights[Playstyle::DefensiveFullBack]      = 20;
        weights[Playstyle::AttackingFullBack]      = 35;
        weights[Playstyle::InvertedWingBack]       = 20;
        weights[Playstyle::BoxToBox]               = 15;  // hybrid
    } else if (pos == "DM") {
        weights[Playstyle::AnchorMan]              = 30;
        weights[Playstyle::DeepLyingPlaymaker]     = 25;
        weights[Playstyle::BoxToBox]               = 15;
        weights[Playstyle::BallPlayingDefender]    = 15;
        weights[Playstyle::Sweeper]                = 10;
        weights[Playstyle::Stopper]                = 5;
    } else if (pos == "CM") {
        weights[Playstyle::BoxToBox]               = 25;
        weights[Playstyle::DeepLyingPlaymaker]     = 20;
        weights[Playstyle::AdvancedPlaymaker]      = 20;
        weights[Playstyle::AnchorMan]              = 10;
        weights[Playstyle::InsideForward]          = 10;
        weights[Playstyle::Raumdeuter]             = 5;
        weights[Playstyle::DeepLyingForward]       = 5;
        weights[Playstyle::False9]                 = 5;
    } else if (pos == "LM" || pos == "RM") {
        weights[Playstyle::AdvancedPlaymaker]      = 15;
        weights[Playstyle::InsideForward]          = 30;
        weights[Playstyle::Raumdeuter]             = 20;
        weights[Playstyle::DeepLyingPlaymaker]     = 10;
        weights[Playstyle::BoxToBox]               = 10;
        weights[Playstyle::AttackingFullBack]      = 15;
    } else if (pos == "AM") {
        weights[Playstyle::AdvancedPlaymaker]      = 30;
        weights[Playstyle::False9]                 = 20;
        weights[Playstyle::InsideForward]          = 20;
        weights[Playstyle::Raumdeuter]             = 15;
        weights[Playstyle::DeepLyingForward]       = 15;
    } else if (pos == "LW" || pos == "RW") {
        weights[Playstyle::InsideForward]          = 30;
        weights[Playstyle::Raumdeuter]             = 25;
        weights[Playstyle::AdvancedPlaymaker]      = 15;
        weights[Playstyle::False9]                 = 15;
        weights[Playstyle::DeepLyingForward]       = 10;
        weights[Playstyle::Poacher]                = 5;
    } else if (pos == "ST") {
        weights[Playstyle::TargetMan]              = 20;
        weights[Playstyle::Poacher]                = 30;
        weights[Playstyle::False9]                 = 15;
        weights[Playstyle::DeepLyingForward]       = 20;
        weights[Playstyle::InsideForward]          = 10;
        weights[Playstyle::Raumdeuter]             = 5;
    }

    // Pick one
    std::vector<Playstyle> styles;
    std::vector<int> w;
    for (auto& kv : weights) { styles.push_back(kv.first); w.push_back(kv.second); }
    if (!w.empty()) {
        m_playstyle = styles[Utils::weightedRandomIndex(w)];
    }
}

void Player::generateTraits() {
    int numTraits = 0;
    int roll = Utils::randInt(1, 100);
    if (roll <= 25)       numTraits = 0;
    else if (roll <= 60)  numTraits = 1;
    else if (roll <= 85)  numTraits = 2;
    else if (roll <= 96)  numTraits = 3;
    else                  numTraits = 4;

    if (numTraits == 0) return;

    struct Candidate { Trait trait; int weight; bool allowed = true; };
    std::vector<Candidate> pool = {
        {Trait::FinesseShot, 5}, {Trait::PowerShot, 5}, {Trait::LongShotTaker, 5},
        {Trait::CurlsBall, 5}, {Trait::DivesIntoTackles, 4}, {Trait::StaysOnFeet, 4},
        {Trait::EarlyCrosser, 4}, {Trait::TriesKillerBalls, 5}, {Trait::PlaysShortSimplePasses, 5},
        {Trait::GiantThrowIn, 2}, {Trait::InjuryProne, 3}, {Trait::Flair, 4},
        {Trait::TeamPlayer, 4}, {Trait::Selfish, 3}, {Trait::Consistent, 4},
        {Trait::Inconsistent, 3}, {Trait::PenaltySpecialist, 3}, {Trait::SetPieceSpecialist, 3},
        {Trait::OneClubMan, 1}, {Trait::BigMatchPlayer, 3}
    };

    // Position-based restrictions
    if (m_primaryPosition == "GK") {
        for (auto& c : pool) {
            if (c.trait == Trait::FinesseShot || c.trait == Trait::PowerShot ||
                c.trait == Trait::LongShotTaker || c.trait == Trait::CurlsBall ||
                c.trait == Trait::EarlyCrosser || c.trait == Trait::DivesIntoTackles ||
                c.trait == Trait::StaysOnFeet || c.trait == Trait::GiantThrowIn ||
                c.trait == Trait::PenaltySpecialist)
                c.allowed = false;
        }
    }

    std::vector<Trait> available;
    std::vector<int> weights;
    for (auto& c : pool) {
        if (c.allowed) { available.push_back(c.trait); weights.push_back(c.weight); }
    }

    m_traits.clear();
    while (m_traits.size() < (size_t)numTraits && !available.empty()) {
        size_t idx = Utils::weightedRandomIndex(weights);
        Trait t = available[idx];
        m_traits.push_back(t);
        // Remove to avoid duplicates
        auto it = std::find(available.begin(), available.end(), t);
        if (it != available.end()) {
            size_t pos = std::distance(available.begin(), it);
            available.erase(it);
            weights.erase(weights.begin() + pos);
        }
    }
}

void Player::generateHiddenAttributes() {
    m_injuryProneness = Utils::randInt(1, 20);
    m_versatility     = Utils::randInt(1, 20);
}

// ========== JSON SERIALISATION ==========

void nlohmann::adl_serializer<Player>::to_json(json& j, const Player& p) {
    j = json{
        {"id", p.m_uniqueId},
        {"name", p.m_name},
        {"age", p.m_age},
        {"nationality", p.m_nationality},
        {"primaryPosition", p.m_primaryPosition},
        {"secondaryPosition", p.m_secondaryPosition},
        {"shooting", p.m_shooting}, {"passing", p.m_passing}, {"crossing", p.m_crossing},
        {"dribbling", p.m_dribbling}, {"tackling", p.m_tackling}, {"heading", p.m_heading},
        {"positioning", p.m_positioning}, {"composure", p.m_composure}, {"vision", p.m_vision},
        {"workRate", p.m_workRate}, {"leadership", p.m_leadership}, {"decisions", p.m_decisions},
        {"pace", p.m_pace}, {"acceleration", p.m_acceleration}, {"stamina", p.m_stamina},
        {"strength", p.m_strength}, {"agility", p.m_agility},
        {"injuryProneness", p.m_injuryProneness}, {"versatility", p.m_versatility},
        {"potentialCeiling", p.m_potentialCeiling},
        {"playstyle", static_cast<int>(p.m_playstyle)},
        {"traits", json::array()},
        {"squadStatus", static_cast<int>(p.m_squadStatus)},
        {"fitness", p.m_fitness}, {"morale", p.m_morale}, {"form", p.m_form},
        {"injured", p.m_injured}, {"injuryDays", p.m_injuryDaysRemaining},
        {"suspended", p.m_suspended}, {"suspensionDays", p.m_suspensionDaysRemaining},
        {"contractMonths", p.m_contractLengthMonths}, {"weeklyWage", p.m_weeklyWage},
        {"homeGrownNation", p.m_homeGrownNation},
        {"appearances", p.m_appearances}, {"goals", p.m_goals}, {"assists", p.m_assists},
        {"yellowCards", p.m_yellowCards}, {"redCards", p.m_redCards},
        {"cleanSheets", p.m_cleanSheets}, {"avgRating", p.m_avgRating},
        {"last5Ratings", p.m_last5Ratings},
        {"careerApps", p.m_careerApps}, {"careerGoals", p.m_careerGoals},
        {"careerAssists", p.m_careerAssists}, {"careerYellowCards", p.m_careerYellowCards},
        {"careerRedCards", p.m_careerRedCards}, {"careerCleanSheets", p.m_careerCleanSheets},
        {"trophiesWon", p.m_trophiesWon}, {"careerAvgRating", p.m_careerAvgRating}
    };
    for (auto& t : p.m_traits) j["traits"].push_back(static_cast<int>(t));
}

void nlohmann::adl_serializer<Player>::from_json(const json& j, Player& p) {
    j.at("id").get_to(p.m_uniqueId);
    j.at("name").get_to(p.m_name);
    j.at("age").get_to(p.m_age);
    j.at("nationality").get_to(p.m_nationality);
    j.at("primaryPosition").get_to(p.m_primaryPosition);
    j.at("secondaryPosition").get_to(p.m_secondaryPosition);

    p.m_shooting     = j.value("shooting", 10);
    p.m_passing      = j.value("passing", 10);
    p.m_crossing     = j.value("crossing", 10);
    p.m_dribbling    = j.value("dribbling", 10);
    p.m_tackling     = j.value("tackling", 10);
    p.m_heading      = j.value("heading", 10);
    p.m_positioning  = j.value("positioning", 10);
    p.m_composure    = j.value("composure", 10);
    p.m_vision       = j.value("vision", 10);
    p.m_workRate     = j.value("workRate", 10);
    p.m_leadership   = j.value("leadership", 10);
    p.m_decisions    = j.value("decisions", 10);
    p.m_pace         = j.value("pace", 10);
    p.m_acceleration = j.value("acceleration", 10);
    p.m_stamina      = j.value("stamina", 10);
    p.m_strength     = j.value("strength", 10);
    p.m_agility      = j.value("agility", 10);
    p.m_injuryProneness = j.value("injuryProneness", 10);
    p.m_versatility  = j.value("versatility", 10);
    p.m_potentialCeiling = j.value("potentialCeiling", 14);

    int ps = j.value("playstyle", 0);
    p.m_playstyle = static_cast<Playstyle>(ps);

    p.m_traits.clear();
    if (j.contains("traits") && j["traits"].is_array()) {
        for (auto& t : j["traits"]) {
            p.m_traits.push_back(static_cast<Trait>(t.get<int>()));
        }
    }

    int ss = j.value("squadStatus", static_cast<int>(SquadStatus::SquadPlayer));
    p.m_squadStatus = static_cast<SquadStatus>(ss);

    p.m_fitness = j.value("fitness", 100);
    p.m_morale  = j.value("morale", 70);
    p.m_form    = j.value("form", 50);
    p.m_injured = j.value("injured", false);
    p.m_injuryDaysRemaining = j.value("injuryDays", 0);
    p.m_suspended = j.value("suspended", false);
    p.m_suspensionDaysRemaining = j.value("suspensionDays", 0);

    p.m_contractLengthMonths = j.value("contractMonths", 24);
    p.m_weeklyWage = j.value("weeklyWage", 5000);
    p.m_homeGrownNation = j.value("homeGrownNation", "");

    p.m_appearances  = j.value("appearances", 0);
    p.m_goals        = j.value("goals", 0);
    p.m_assists      = j.value("assists", 0);
    p.m_yellowCards  = j.value("yellowCards", 0);
    p.m_redCards     = j.value("redCards", 0);
    p.m_cleanSheets  = j.value("cleanSheets", 0);
    p.m_avgRating    = j.value("avgRating", 0.0);
    if (j.contains("last5Ratings")) j.at("last5Ratings").get_to(p.m_last5Ratings);

    p.m_careerApps    = j.value("careerApps", 0);
    p.m_careerGoals   = j.value("careerGoals", 0);
    p.m_careerAssists = j.value("careerAssists", 0);
    p.m_careerYellowCards = j.value("careerYellowCards", 0);
    p.m_careerRedCards    = j.value("careerRedCards", 0);
    p.m_careerCleanSheets = j.value("careerCleanSheets", 0);
    p.m_trophiesWon   = j.value("trophiesWon", 0);
    p.m_careerAvgRating = j.value("careerAvgRating", 0.0);
}
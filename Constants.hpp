#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace FM {
    // The Match Engine Stage
    constexpr int GRID_X = 12;
    constexpr int GRID_Y = 8;

    // The Engine Fuel (Strict 1-20 Scale)
    constexpr int MIN_ATTR = 1;
    constexpr int MAX_ATTR = 20;
    constexpr int MAX_REPUTATION = 20;
    
    // Hidden Editor Bounds
    constexpr int MIN_CA_PA = 1;
    constexpr int MAX_CA_PA = 200;

    // Match Constants
    constexpr int MATCH_DURATION = 90;
    constexpr float TICK_RATE = 0.5f; // Engine calculates every 0.5 in-game seconds
    
    // The "Skin in the Game" Metrics
    constexpr int CONFIDENCE_SACK_THRESHOLD = 15;
    constexpr int CONFIDENCE_STARTING = 60;
    constexpr int CONFIDENCE_MAX = 100;
}

#endif
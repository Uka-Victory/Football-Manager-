#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

class Utils {
public:
    // Initializes the random number generator (Call once in main)
    static void initRNG();
    
    // Returns a random integer between min and max (inclusive)
    static int randInt(int min, int max);

    // Generates a random 8-character hex ID (e.g., "P_A4F9C2B1") to prevent collisions
    static std::string generateUniqueId(const std::string& prefix = "P_");
};

#endif
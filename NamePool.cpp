#include "NamePool.hpp"
#include "Utils.hpp"

namespace FootballManager {

    NamePool::NamePool() {
        firstNames = {"James", "Ahmed", "Carlos", "David", "Sato",
                      "Marco", "Luis",  "Pierre", "Emeka", "Yuki"};
        lastNames  = {"Smith",  "Johnson", "Silva",   "Müller", "Okafor",
                      "Rossi",  "Herrera", "Dubois",  "Adeola", "Tanaka"};
        countries  = {"England", "Brazil",    "Germany", "Nigeria", "Japan",
                      "Italy",   "Argentina", "France",  "Ghana",   "Spain"};
    }

    // Fix: Use Utils::randInt (properly seeded) instead of a local random_device per call.
    // The nation parameter is reserved for future culturally-aware name lookup.
    std::string NamePool::generateName(const std::string& /*nation*/) {
        if (firstNames.empty() || lastNames.empty()) return "Unknown Player";

        int firstIdx = Utils::randInt(0, static_cast<int>(firstNames.size()) - 1);
        int lastIdx  = Utils::randInt(0, static_cast<int>(lastNames.size())  - 1);

        return firstNames[firstIdx] + " " + lastNames[lastIdx];
    }

    std::string NamePool::getRandomCountry() {
        if (countries.empty()) return "Unknown";
        return countries[Utils::randInt(0, static_cast<int>(countries.size()) - 1)];
    }

} // namespace FootballManager
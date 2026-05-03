#pragma once
#include "Player.hpp"
#include "Finances.hpp"
#include "Facilities.hpp"
#include <vector>
#include <string>
#include <memory>

namespace FootballManager {

    class Team {
    private:
        std::string name;
        int clubLevel;
        std::vector<std::shared_ptr<Player>> seniorSquad;
        std::shared_ptr<Finances>   finances;
        std::shared_ptr<Facilities> facilities;

    public:
        Team(std::string teamName, int level);

        std::string getName()     const { return name; }
        int getClubLevel()        const { return clubLevel; }

        std::shared_ptr<Finances>   getFinances()   const { return finances; }
        std::shared_ptr<Facilities> getFacilities() const { return facilities; }

        void addPlayerToSenior(std::shared_ptr<Player> player);
        void releasePlayer(std::shared_ptr<Player> player);

        std::vector<std::shared_ptr<Player>> getSeniorSquad() const { return seniorSquad; }

        // Fix: getBestXI now guarantees a GK at index 0 before filling outfield spots
        std::vector<std::shared_ptr<Player>> getBestXI();
    };

} // namespace FootballManager
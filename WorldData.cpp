// WorldData.cpp
#include "WorldData.hpp"
#include <fstream>
#include <iostream>

bool WorldData::load(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return false;
    json j;
    in >> j;

    m_countries.clear();
    if (!j.contains("countries") || !j["countries"].is_array()) return false;

    for (const auto& cj : j["countries"]) {
        CountryInfo ci;
        ci.name = cj.at("name").get<std::string>();
        if (cj.contains("leagues")) {
            for (const auto& lj : cj["leagues"]) {
                LeagueInfo li;
                li.name = lj.at("name").get<std::string>();
                li.tier = lj.value("tier", 1);
                li.level = lj.value("level", 10);
                li.roundsPerOpponent = lj.value("roundsPerOpponent", 2);
                li.promotionSpots = lj.value("promotionSpots", 3);
                li.relegationSpots = lj.value("relegationSpots", 3);
                if (lj.contains("teams")) {
                    for (const auto& tj : lj["teams"]) {
                        TeamInfo ti;
                        ti.name = tj.at("name").get<std::string>();
                        ti.level = tj.value("level", li.level);
                        ti.country = ci.name;
                        ti.seniorCount = 22;
                        ti.youthCount = 8;
                        ti.primaryColour = tj.value("primaryColour", "Red");
                        ti.secondaryColour = tj.value("secondaryColour", "White");
                        ti.stadium = tj.value("stadium", ti.name + " Stadium");
                        ti.founded = tj.value("founded", 1900);
                        li.teams.push_back(ti);
                    }
                }
                ci.leagues.push_back(li);
            }
        }
        m_countries.push_back(ci);
    }
    return !m_countries.empty();
}

const std::vector<CountryInfo>& WorldData::getBaseCountries() const {
    return m_countries;
}
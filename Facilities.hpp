// Facilities.hpp
#pragma once
#include "json.hpp"
struct Facilities {
    int training = 1;
    int academy = 1;
    int stadium = 1;
    int infirmary = 1;
    int subLevels[4] = {0,0,0,0};

    void degrade(int years = 1);
    json toJson() const;
    static Facilities fromJson(const json& j);
};
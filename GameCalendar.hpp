// GameCalendar.hpp
#pragma once
#include <string>
#include "json.hpp"
using json = nlohmann::json;

class GameCalendar {
public:
    GameCalendar(int y = 2025, int m = 7, int d = 1);
    void advanceOneDay();
    std::string getDateString() const;
    int getYear() const;
    int getWeekday() const; // 0=Sunday
    bool isSeasonEnd() const; // June 30
    json toJson() const;
    void fromJson(const json& j);
private:
    int year, month, day;
    static int daysInMonth(int m, int y);
};
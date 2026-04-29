#ifndef GAMECALENDAR_HPP
#define GAMECALENDAR_HPP

#include <string>
#include "json.hpp"

class GameCalendar {
private:
    int year;
    int month;
    int day;
    
    // Internal safety checks to prevent invalid dates
    int getDaysInMonth(int m, int y) const;
    bool isLeapYear(int y) const;

public:
    GameCalendar();
    GameCalendar(int startYear, int startMonth, int startDay);

    // Progression
    void advanceDay();
    void advanceDays(int days);

    // Getters
    int getYear() const;
    int getMonth() const;
    int getDay() const;
    std::string getDateString() const;
    
    // Season Rollover Trigger
    bool isSeasonEnd() const; // Returns true if the date hits June 30th

    // Save/Load System
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

#endif
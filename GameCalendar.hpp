#ifndef GAMECALENDAR_HPP
#define GAMECALENDAR_HPP

#include <string>

class GameCalendar {
public:
    int currentDay;
    int currentMonth;
    int currentYear;
    int currentWeekday;   // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
    bool summerWindowOpen;
    bool winterWindowOpen;
    
    GameCalendar();
    void advanceOneDay();
    std::string getDateString() const;
    bool isTransferWindowOpen() const;
    bool isSunday() const;
    bool isSeasonEnd() const;
};

#endif
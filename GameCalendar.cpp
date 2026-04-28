#include "GameCalendar.hpp"

GameCalendar::GameCalendar() 
    : currentDay(1), currentMonth(7), currentYear(2025),
      currentWeekday(2),  // July 1, 2025 was a Tuesday (2 = Tuesday if 0=Sunday? Let's use 0=Monday for simplicity? Actually I'll keep 0=Sunday convention)
      summerWindowOpen(true), winterWindowOpen(false) {
    // Let's define: 0=Sunday,1=Monday,2=Tuesday,3=Wednesday,4=Thursday,5=Friday,6=Saturday
    // July 1, 2025 is Tuesday -> weekday = 2
}

void GameCalendar::advanceOneDay() {
    currentDay++;
    currentWeekday = (currentWeekday + 1) % 7;
    
    // Get days in current month
    int daysInMonth;
    if (currentMonth == 2) daysInMonth = 28;          // February (ignore leap year for simplicity)
    else if (currentMonth == 4 || currentMonth == 6 || currentMonth == 9 || currentMonth == 11) daysInMonth = 30;
    else daysInMonth = 31;
    
    if (currentDay > daysInMonth) {
        currentDay = 1;
        currentMonth++;
        if (currentMonth > 12) {
            currentMonth = 1;
            currentYear++;
        }
    }
    
    // Summer transfer window: June 15 - August 31
    if (currentMonth == 6 && currentDay >= 15) summerWindowOpen = true;
    if (currentMonth == 9 && currentDay >= 1) summerWindowOpen = false;
    
    // Winter transfer window: January 1 - January 31
    if (currentMonth == 1 && currentDay >= 1) winterWindowOpen = true;
    if (currentMonth == 2 && currentDay >= 1) winterWindowOpen = false;
}

std::string GameCalendar::getDateString() const {
    std::string year = std::to_string(currentYear);
    std::string month = (currentMonth < 10) ? "0" + std::to_string(currentMonth) : std::to_string(currentMonth);
    std::string day = (currentDay < 10) ? "0" + std::to_string(currentDay) : std::to_string(currentDay);
    return year + "-" + month + "-" + day;
}

bool GameCalendar::isTransferWindowOpen() const {
    return summerWindowOpen || winterWindowOpen;
}

bool GameCalendar::isSunday() const {
    return currentWeekday == 0;
}

bool GameCalendar::isSeasonEnd() const {
    return currentMonth == 5 && currentDay >= 31;
}
#include "GameCalendar.hpp"
#include <sstream>
#include <iomanip>

// Default start date: July 1st, 2024
GameCalendar::GameCalendar() : year(2024), month(7), day(1) {} 

GameCalendar::GameCalendar(int startYear, int startMonth, int startDay) 
    : year(startYear), month(startMonth), day(startDay) {}

bool GameCalendar::isLeapYear(int y) const {
    // A year is a leap year if it is divisible by 4, 
    // except for end-of-century years which must be divisible by 400.
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int GameCalendar::getDaysInMonth(int m, int y) const {
    switch (m) {
        case 4: case 6: case 9: case 11: // Thirty days hath September, April, June, and November...
            return 30;
        case 2: // February
            return isLeapYear(y) ? 29 : 28;
        default: // All the rest have 31
            return 31;
    }
}

void GameCalendar::advanceDay() {
    day++;
    if (day > getDaysInMonth(month, year)) {
        day = 1;
        month++;
        if (month > 12) {
            month = 1;
            year++;
        }
    }
}

void GameCalendar::advanceDays(int days) {
    for (int i = 0; i < days; ++i) {
        advanceDay();
    }
}

int GameCalendar::getYear() const { return year; }
int GameCalendar::getMonth() const { return month; }
int GameCalendar::getDay() const { return day; }

std::string GameCalendar::getDateString() const {
    std::stringstream ss;
    // Format visually as YYYY-MM-DD
    ss << year << "-" 
       << std::setw(2) << std::setfill('0') << month << "-" 
       << std::setw(2) << std::setfill('0') << day;
    return ss.str();
}

bool GameCalendar::isSeasonEnd() const {
    // The global football season ends on June 30th.
    return (month == 6 && day == 30);
}

// --- SAVE / LOAD SYSTEM ---

nlohmann::json GameCalendar::toJson() const {
    return {
        {"year", year},
        {"month", month},
        {"day", day}
    };
}

void GameCalendar::fromJson(const nlohmann::json& j) {
    // If the save file is missing a date for some reason, default to start date
    year = j.value("year", 2024);
    month = j.value("month", 7);
    day = j.value("day", 1);
}
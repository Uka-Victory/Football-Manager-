// GameCalendar.cpp
#include "GameCalendar.hpp"
#include <sstream>
#include <iomanip>

GameCalendar::GameCalendar(int y, int m, int d) : year(y), month(m), day(d) {}

void GameCalendar::advanceOneDay() {
    day++;
    if (day > daysInMonth(month, year)) { day = 1; month++; }
    if (month > 12) { month = 1; year++; }
}

std::string GameCalendar::getDateString() const {
    std::ostringstream oss;
    oss << year << "-" << std::setfill('0') << std::setw(2) << month
        << "-" << std::setw(2) << day;
    return oss.str();
}

int GameCalendar::getYear() const { return year; }

int GameCalendar::getWeekday() const {
    // Simple Zeller-like algorithm for 2025-01-01 = Wednesday (3)
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    int y = year - (month < 3);
    return (y + y/4 - y/100 + y/400 + t[month-1] + day) % 7;
}

bool GameCalendar::isSeasonEnd() const {
    return (month == 6 && day == 30);
}

int GameCalendar::daysInMonth(int m, int y) {
    static const int md[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int d = md[m-1];
    if (m == 2 && (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0))) d = 29;
    return d;
}

json GameCalendar::toJson() const {
    return {{"year", year}, {"month", month}, {"day", day}};
}

void GameCalendar::fromJson(const json& j) {
    year = j.at("year"); month = j.at("month"); day = j.at("day");
}
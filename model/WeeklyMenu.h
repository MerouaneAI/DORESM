#pragma once
#include <string>
#include <vector>

struct DailyMenu {
    std::string breakfast;
    std::string lunch;
    std::string dinner;
    int mealsServed = 0;
};

struct WeeklyMenu {
    DailyMenu days[7]; // 0 = Monday, ..., 6 = Sunday
};

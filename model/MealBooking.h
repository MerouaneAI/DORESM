#pragma once
#include <string>
#include <utility>
#include "Common.h"

struct MealBooking {
    std::string studentId;
    std::string date;     // YYYY-MM-DD
    MealType    meal = MealType::Lunch;
    bool        confirmed = true;
    bool        served = false;
    MealBooking() = default;
    MealBooking(std::string s, std::string d, MealType m, bool c = true, bool sv = false)
        : studentId(std::move(s)), date(std::move(d)), meal(m), confirmed(c), served(sv) {}
};
#include "Common.h"

std::string mealTypeToString(MealType t) {
    switch (t) {
        case MealType::Breakfast: return "Breakfast";
        case MealType::Lunch:     return "Lunch";
        case MealType::Dinner:    return "Dinner";
    }
    return "Lunch";
}

MealType mealTypeFromString(const std::string& s) {
    if (s == "Breakfast") return MealType::Breakfast;
    if (s == "Dinner")    return MealType::Dinner;
    return MealType::Lunch;
}
#pragma once
#include <string>

enum class MealType { Breakfast, Lunch, Dinner };

std::string mealTypeToString(MealType t);
MealType    mealTypeFromString(const std::string& s);
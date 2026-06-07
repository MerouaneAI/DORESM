#pragma once
#include <string>
#include <utility>
#include "Menu.h"

class Restaurant {
private:
    std::string name;
    Menu todayMenu;
    int  mealsServedToday = 0;
public:
    Restaurant() = default;
    explicit Restaurant(std::string name) : name(std::move(name)) {}

    const std::string& getName() const { return name; }
    void setName(const std::string& n) { name = n; }

    const Menu& getMenu() const { return todayMenu; }
    void setMenu(const Menu& m) { todayMenu = m; }

    int  getMealsServed() const { return mealsServedToday; }
    void serveMeal() { ++mealsServedToday; }
    void resetMeals() { mealsServedToday = 0; }
};
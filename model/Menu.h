#pragma once
#include <string>
#include <utility>

struct Menu {
    std::string breakfast;
    std::string lunch;
    std::string dinner;
    Menu() = default;
    Menu(std::string b, std::string l, std::string d)
        : breakfast(std::move(b)), lunch(std::move(l)), dinner(std::move(d)) {}
};
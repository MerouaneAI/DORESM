#include "Activity.h"
#include <algorithm>
#include <stdexcept>

void Activity::enroll(const std::string& studentId) {
    if (std::find(participants.begin(), participants.end(), studentId) != participants.end())
        throw std::runtime_error("Student already enrolled in " + name);
    participants.push_back(studentId);
}

void Activity::unenroll(const std::string& studentId) {
    auto it = std::find(participants.begin(), participants.end(), studentId);
    if (it == participants.end())
        throw std::runtime_error("Student not enrolled in " + name);
    participants.erase(it);
}

std::string Activity::describe() const {
    return category() + " - " + name + " (" + schedule + ")";
}

std::string SportsActivity::describe() const {
    return "Sports - " + name + " (" + schedule + "), Coach: " + coach;
}

std::string CulturalActivity::describe() const {
    return "Cultural - " + name + " (" + schedule + "), Venue: " + venue;
}
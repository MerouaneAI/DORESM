#include "Activity.h"
#include <algorithm>
#include <stdexcept>

void Activity::apply(const std::string& studentId) {
    if (hasEnrollment(studentId))
        throw std::runtime_error("Student already has an application for " + name);
    enrollments.emplace_back(studentId, "Pending");
    rebuildParticipants();
}

void Activity::approve(const std::string& studentId) {
    for (auto& e : enrollments) {
        if (e.studentId == studentId) {
            e.status = "Approved";
            rebuildParticipants();
            return;
        }
    }
    throw std::runtime_error("No application found for student in " + name);
}

void Activity::refuse(const std::string& studentId) {
    for (auto& e : enrollments) {
        if (e.studentId == studentId) {
            e.status = "Refused";
            rebuildParticipants();
            return;
        }
    }
    throw std::runtime_error("No application found for student in " + name);
}

void Activity::withdraw(const std::string& studentId) {
    auto it = std::find_if(enrollments.begin(), enrollments.end(),
        [&](const Enrollment& e){ return e.studentId == studentId; });
    if (it == enrollments.end())
        throw std::runtime_error("Student not enrolled in " + name);
    enrollments.erase(it);
    rebuildParticipants();
}

// Legacy: admin directly enrolls → Approved
void Activity::enroll(const std::string& studentId) {
    if (hasEnrollment(studentId)) {
        // If pending, approve instead
        for (auto& e : enrollments) {
            if (e.studentId == studentId) {
                e.status = "Approved";
                rebuildParticipants();
                return;
            }
        }
    }
    enrollments.emplace_back(studentId, "Approved");
    rebuildParticipants();
}

void Activity::unenroll(const std::string& studentId) {
    withdraw(studentId);
}

bool Activity::hasEnrollment(const std::string& studentId) const {
    return std::any_of(enrollments.begin(), enrollments.end(),
        [&](const Enrollment& e){ return e.studentId == studentId; });
}

std::string Activity::enrollmentStatus(const std::string& studentId) const {
    for (const auto& e : enrollments)
        if (e.studentId == studentId) return e.status;
    return "";
}

void Activity::rebuildParticipants() {
    participants.clear();
    for (const auto& e : enrollments)
        if (e.status == "Approved")
            participants.push_back(e.studentId);
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
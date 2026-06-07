#include "Student.h"
#include <stdexcept>

Student::Student(std::string id, std::string fullName, int academicYear)
    : Person(std::move(id), std::move(fullName)), academicYear(academicYear) {
    if (academicYear < 1 || academicYear > 5)
        throw std::invalid_argument("Academic year must be between 1 and 5");
}

void Student::setAcademicYear(int y) {
    if (y < 1 || y > 5)
        throw std::invalid_argument("Academic year must be between 1 and 5");
    academicYear = y;
}

void Student::assignAccommodation(const std::string& dormId, const std::string& room) {
    dormitoryId = dormId;
    roomNumber  = room;
}

void Student::clearAccommodation() {
    dormitoryId.clear();
    roomNumber.clear();
}

std::string Student::accommodationStatus() const {
    return isAccommodated()
        ? ("Dorm " + dormitoryId + " / Room " + roomNumber)
        : "Not assigned";
}
#pragma once
#include "Person.h"
#include <string>

class Student : public Person {
private:
    int academicYear = 1;          // 1..5
    std::string dormitoryId;       // empty if unassigned
    std::string roomNumber;        // empty if unassigned
public:
    Student() = default;
    Student(std::string id, std::string fullName, int academicYear);

    int  getAcademicYear() const { return academicYear; }
    void setAcademicYear(int y);

    const std::string& getDormitoryId() const { return dormitoryId; }
    const std::string& getRoomNumber() const { return roomNumber; }
    bool isAccommodated() const { return !roomNumber.empty(); }

    void assignAccommodation(const std::string& dormId, const std::string& room);
    void clearAccommodation();

    std::string role() const override { return "Student"; }
    std::string accommodationStatus() const;
};
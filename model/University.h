#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Student.h"
#include "Dormitory.h"
#include "HealthClinic.h"
#include "Activity.h"
#include "MealBooking.h"

class University {
private:
    std::string name;
    std::vector<Student>   students;
    std::vector<Dormitory> dormitories;
    HealthClinic           clinic;
    std::vector<std::unique_ptr<Activity>> activities; // polymorphic ownership
    std::vector<MealBooking>               bookings;
public:
    explicit University(std::string name = "ENSIA") : name(std::move(name)) {}

    const std::string& getName() const { return name; }

    // ----- Students -----
    std::vector<Student>&       getStudents()       { return students; }
    const std::vector<Student>& getStudents() const { return students; }
    Student* findStudent(const std::string& id);
    void addStudent(const Student& s);
    void removeStudent(const std::string& id);
    std::string studentName(const std::string& id) const;

    // ----- Dormitories -----
    std::vector<Dormitory>&       getDormitories()       { return dormitories; }
    const std::vector<Dormitory>& getDormitories() const { return dormitories; }
    Dormitory* findDormitory(const std::string& id);
    void addDormitory(const Dormitory& d);

    // ----- Accommodation -----
    void assignStudentToRoom(const std::string& studentId,
                             const std::string& dormId,
                             const std::string& roomNumber);
    void removeStudentFromRoom(const std::string& studentId);

    // ----- Added services -----
    HealthClinic& getClinic() { return clinic; }
    std::vector<std::unique_ptr<Activity>>& getActivities() { return activities; }
    void addActivity(std::unique_ptr<Activity> a) { activities.push_back(std::move(a)); }
    std::vector<MealBooking>& getBookings() { return bookings; }
    void bookMeal(const MealBooking& b);

    // ----- Stats -----
    int totalStudents()  const { return static_cast<int>(students.size()); }
    int totalRooms()     const;
    int availableRooms() const;
    int occupiedRooms()  const;

    // ----- Persistence -----
    void saveToFiles(const std::string& dir) const;
    void loadFromFiles(const std::string& dir);
    void seedSampleData();
};
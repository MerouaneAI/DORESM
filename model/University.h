#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Student.h"
#include "Dormitory.h"
#include "HealthClinic.h"
#include "Activity.h"
#include "MealBooking.h"
#include "WeeklyMenu.h"

struct ActivityLogItem {
    std::string emoji;
    std::string text;
    std::string time;   // "HH:MM"
};

class University {
private:
    std::string name;
    std::vector<Student>   students;
    std::vector<Dormitory> dormitories;
    HealthClinic           clinic;
    std::vector<std::unique_ptr<Activity>> activities; // polymorphic ownership
    std::vector<MealBooking>               bookings;
    std::vector<ActivityLogItem> activityLog;
    WeeklyMenu weeklyMenu;
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
    void reassignStudent(const std::string& studentId,
                         const std::string& newDormId,
                         const std::string& newRoomNumber);

    // ----- Added services -----
    HealthClinic& getClinic() { return clinic; }
    std::vector<std::unique_ptr<Activity>>& getActivities() { return activities; }
    void addActivity(std::unique_ptr<Activity> a) { activities.push_back(std::move(a)); }
    std::vector<MealBooking>& getBookings() { return bookings; }
    void bookMeal(const MealBooking& b);

    // ----- Weekly Menu -----
    WeeklyMenu& getWeeklyMenu() { return weeklyMenu; }
    const WeeklyMenu& getWeeklyMenu() const { return weeklyMenu; }

    // ----- Stats -----
    int totalStudents()  const { return static_cast<int>(students.size()); }
    int totalRooms()     const;
    int availableRooms() const;
    int occupiedRooms()  const;

    // ----- Persistence -----
    void saveToFiles(const std::string& dir) const;
    void loadFromFiles(const std::string& dir);
    void seedSampleData();

    // ----- Activity log (drives the dashboard "Recent Activity") -----
const std::vector<ActivityLogItem>& getActivityLog() const { return activityLog; }
void logActivity(const std::string& emoji, const std::string& text);

// ----- Dormitories (new) -----
void removeDormitory(const std::string& id);   // unassigns residents, then deletes
};
#pragma once
#include <string>
#include <vector>
#include <utility>

// Enrollment status for activity applications
struct Enrollment {
    std::string studentId;
    std::string status;  // "Pending" | "Approved" | "Refused"
    Enrollment() = default;
    Enrollment(std::string sid, std::string st = "Pending")
        : studentId(std::move(sid)), status(std::move(st)) {}
};

class Activity {
protected:
    std::string name;
    std::string schedule;
    std::vector<Enrollment> enrollments;
    // Legacy participants kept for backward compat during migration
    std::vector<std::string> participants;
public:
    Activity() = default;
    Activity(std::string name, std::string schedule)
        : name(std::move(name)), schedule(std::move(schedule)) {}
    virtual ~Activity() = default;

    const std::string& getName() const { return name; }
    const std::string& getSchedule() const { return schedule; }

    // Legacy — returns approved + pending IDs for backward compat
    const std::vector<std::string>& getParticipants() const { return participants; }

    // New enrollment system
    const std::vector<Enrollment>& getEnrollments() const { return enrollments; }
    std::vector<Enrollment>& getEnrollments() { return enrollments; }

    void apply(const std::string& studentId);       // student applies (status = Pending)
    void approve(const std::string& studentId);      // admin approves
    void refuse(const std::string& studentId);       // admin refuses
    void withdraw(const std::string& studentId);     // student withdraws application

    // Legacy enroll/unenroll — these now create Approved enrollments directly (admin use)
    void enroll(const std::string& studentId);
    void unenroll(const std::string& studentId);

    // Helpers
    bool hasEnrollment(const std::string& studentId) const;
    std::string enrollmentStatus(const std::string& studentId) const;

    // Rebuild participants list from enrollments (called after changes)
    void rebuildParticipants();

    virtual std::string category() const = 0;   // polymorphic
    virtual std::string describe() const;        // overridable
};

class SportsActivity : public Activity {
private:
    std::string coach;
public:
    SportsActivity() = default;
    SportsActivity(std::string name, std::string schedule, std::string coach)
        : Activity(std::move(name), std::move(schedule)), coach(std::move(coach)) {}
    const std::string& getCoach() const { return coach; }
    std::string category() const override { return "Sports"; }
    std::string describe() const override;
};

class CulturalActivity : public Activity {
private:
    std::string venue;
public:
    CulturalActivity() = default;
    CulturalActivity(std::string name, std::string schedule, std::string venue)
        : Activity(std::move(name), std::move(schedule)), venue(std::move(venue)) {}
    const std::string& getVenue() const { return venue; }
    std::string category() const override { return "Cultural"; }
    std::string describe() const override;
};
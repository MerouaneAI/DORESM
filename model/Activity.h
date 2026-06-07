#pragma once
#include <string>
#include <vector>
#include <utility>

class Activity {
protected:
    std::string name;
    std::string schedule;
    std::vector<std::string> participants; // student IDs
public:
    Activity() = default;
    Activity(std::string name, std::string schedule)
        : name(std::move(name)), schedule(std::move(schedule)) {}
    virtual ~Activity() = default;

    const std::string& getName() const { return name; }
    const std::string& getSchedule() const { return schedule; }
    const std::vector<std::string>& getParticipants() const { return participants; }

    void enroll(const std::string& studentId);
    void unenroll(const std::string& studentId);

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
#pragma once
#include <string>
#include <vector>
#include <utility>

struct Appointment {
    std::string studentId;
    std::string date;    // YYYY-MM-DD
    std::string time;    // HH:MM
    std::string reason;
    std::string status;  // Scheduled / Completed / Cancelled
    Appointment() = default;
    Appointment(std::string s, std::string d, std::string t, std::string r,
                std::string st = "Scheduled")
        : studentId(std::move(s)), date(std::move(d)), time(std::move(t)),
          reason(std::move(r)), status(std::move(st)) {}
};

class HealthClinic {
private:
    std::vector<Appointment> appointments;
public:
    const std::vector<Appointment>& getAppointments() const { return appointments; }
    std::vector<Appointment>&       getAppointments()       { return appointments; }
    void schedule(const Appointment& a);
    void cancel(size_t index);
};
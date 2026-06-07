#include "HealthClinic.h"
#include <stdexcept>

void HealthClinic::schedule(const Appointment& a) {
    if (a.studentId.empty())
        throw std::runtime_error("Appointment requires a student");
    appointments.push_back(a);
}

void HealthClinic::cancel(size_t index) {
    if (index >= appointments.size())
        throw std::out_of_range("Invalid appointment index");
    appointments[index].status = "Cancelled";
}
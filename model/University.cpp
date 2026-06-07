#include "University.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>

namespace {
    std::vector<std::string> split(const std::string& line, char delim) {
        std::vector<std::string> out;
        std::string cur;
        std::stringstream ss(line);
        while (std::getline(ss, cur, delim)) out.push_back(cur);
        if (!line.empty() && line.back() == delim) out.push_back("");
        return out;
    }
}

// ---------- Students ----------
Student* University::findStudent(const std::string& id) {
    for (auto& s : students) if (s.getId() == id) return &s;
    return nullptr;
}

std::string University::studentName(const std::string& id) const {
    for (const auto& s : students) if (s.getId() == id) return s.getFullName();
    return id;
}

void University::addStudent(const Student& s) {
    if (s.getId().empty()) throw std::runtime_error("Student ID is required");
    if (findStudent(s.getId())) throw std::runtime_error("Duplicate student ID: " + s.getId());
    students.push_back(s);
}

void University::removeStudent(const std::string& id) {
    Student* s = findStudent(id);
    if (!s) throw std::runtime_error("Student not found: " + id);
    if (s->isAccommodated()) removeStudentFromRoom(id);
    students.erase(std::remove_if(students.begin(), students.end(),
        [&](const Student& x){ return x.getId() == id; }), students.end());
}

// ---------- Dormitories ----------
Dormitory* University::findDormitory(const std::string& id) {
    for (auto& d : dormitories) if (d.getId() == id) return &d;
    return nullptr;
}

void University::addDormitory(const Dormitory& d) {
    if (findDormitory(d.getId())) throw std::runtime_error("Duplicate dormitory: " + d.getId());
    dormitories.push_back(d);
}

// ---------- Accommodation ----------
void University::assignStudentToRoom(const std::string& studentId,
                                     const std::string& dormId,
                                     const std::string& roomNumber) {
    Student* s = findStudent(studentId);
    if (!s) throw std::runtime_error("Student not found: " + studentId);
    if (s->isAccommodated()) throw std::runtime_error("Student is already accommodated");
    Dormitory* d = findDormitory(dormId);
    if (!d) throw std::runtime_error("Dormitory not found: " + dormId);
    d->assignStudent(roomNumber, studentId);          // may throw (capacity / maintenance)
    s->assignAccommodation(dormId, roomNumber);
}

void University::removeStudentFromRoom(const std::string& studentId) {
    Student* s = findStudent(studentId);
    if (!s) throw std::runtime_error("Student not found: " + studentId);
    if (!s->isAccommodated()) return;
    if (Dormitory* d = findDormitory(s->getDormitoryId()))
        d->removeStudent(s->getRoomNumber(), studentId);
    s->clearAccommodation();
}

void University::bookMeal(const MealBooking& b) {
    if (!findStudent(b.studentId)) throw std::runtime_error("Unknown student for booking");
    bookings.push_back(b);
}

// ---------- Stats ----------
int University::totalRooms() const {
    int t = 0; for (const auto& d : dormitories) t += static_cast<int>(d.getRooms().size()); return t;
}
int University::availableRooms() const {
    int t = 0;
    for (const auto& d : dormitories)
        for (const auto& r : d.getRooms())
            if (!r.isFull() && !r.isUnderMaintenance()) ++t;
    return t;
}
int University::occupiedRooms() const {
    int t = 0;
    for (const auto& d : dormitories)
        for (const auto& r : d.getRooms())
            if (!r.isEmpty()) ++t;
    return t;
}

// ---------- Persistence ----------
void University::saveToFiles(const std::string& dir) const {
    std::filesystem::create_directories(dir);

    std::ofstream fs(dir + "/students.txt");
    for (const auto& s : students)
        fs << s.getId() << '|' << s.getFullName() << '|' << s.getAcademicYear()
           << '|' << s.getDormitoryId() << '|' << s.getRoomNumber() << '\n';

    std::ofstream fd(dir + "/dormitories.txt");
    for (const auto& d : dormitories) {
        fd << "DORM|" << d.getId() << '|' << d.getName() << '\n';
        for (const auto& r : d.getRooms()) {
            fd << "ROOM|" << r.getNumber() << '|' << r.getCapacity() << '|' << r.getType()
               << '|' << (r.isUnderMaintenance() ? 1 : 0) << '|';
            const auto& occ = r.getOccupants();
            for (size_t i = 0; i < occ.size(); ++i) { fd << occ[i]; if (i + 1 < occ.size()) fd << ','; }
            fd << '\n';
        }
        const Menu& m = d.getRestaurant().getMenu();
        fd << "MENU|" << m.breakfast << '|' << m.lunch << '|' << m.dinner
           << '|' << d.getRestaurant().getMealsServed() << '\n';
    }

    std::ofstream fb(dir + "/bookings.txt");
    for (const auto& b : bookings)
        fb << b.studentId << '|' << b.date << '|' << mealTypeToString(b.meal)
           << '|' << (b.confirmed ? 1 : 0) << '\n';

    std::ofstream fa(dir + "/activities.txt");
    for (const auto& a : activities) {
        std::string extra;
        if (auto* sp = dynamic_cast<SportsActivity*>(a.get()))   extra = sp->getCoach();
        else if (auto* cu = dynamic_cast<CulturalActivity*>(a.get())) extra = cu->getVenue();
        fa << a->category() << '|' << a->getName() << '|' << a->getSchedule() << '|' << extra << '|';
        const auto& p = a->getParticipants();
        for (size_t i = 0; i < p.size(); ++i) { fa << p[i]; if (i + 1 < p.size()) fa << ','; }
        fa << '\n';
    }

    std::ofstream fc(dir + "/appointments.txt");
    for (const auto& ap : clinic.getAppointments())
        fc << ap.studentId << '|' << ap.date << '|' << ap.time << '|'
           << ap.reason << '|' << ap.status << '\n';
}

void University::loadFromFiles(const std::string& dir) {
    students.clear(); dormitories.clear(); bookings.clear();
    activities.clear(); clinic.getAppointments().clear();
    std::string line;

    std::ifstream fd(dir + "/dormitories.txt");
    Dormitory* current = nullptr;
    while (std::getline(fd, line)) {
        if (line.empty()) continue;
        auto t = split(line, '|');
        if (t[0] == "DORM") {
            dormitories.emplace_back(t[1], t[2]);
            current = &dormitories.back();
        } else if (t[0] == "ROOM" && current) {
            Room r(t[1], std::stoi(t[2]), t[3]);
            if (t.size() > 5 && !t[5].empty())
                for (auto& occ : split(t[5], ',')) if (!occ.empty()) r.addOccupant(occ);
            if (t.size() > 4 && t[4] == "1") r.setMaintenance(true);
            current->addRoom(r);
        } else if (t[0] == "MENU" && current) {
            current->getRestaurant().setMenu(Menu(t[1], t[2], t[3]));
            if (t.size() > 4) { int n = std::stoi(t[4]); for (int i = 0; i < n; ++i) current->getRestaurant().serveMeal(); }
        }
    }

    std::ifstream fs(dir + "/students.txt");
    while (std::getline(fs, line)) {
        if (line.empty()) continue;
        auto t = split(line, '|');
        Student s(t[0], t[1], std::stoi(t[2]));
        if (t.size() > 4 && !t[4].empty()) s.assignAccommodation(t[3], t[4]);
        students.push_back(s);
    }

    std::ifstream fb(dir + "/bookings.txt");
    while (std::getline(fb, line)) {
        if (line.empty()) continue;
        auto t = split(line, '|');
        bookings.emplace_back(t[0], t[1], mealTypeFromString(t[2]), t.size() > 3 && t[3] == "1");
    }

    std::ifstream fa(dir + "/activities.txt");
    while (std::getline(fa, line)) {
        if (line.empty()) continue;
        auto t = split(line, '|');
        std::unique_ptr<Activity> a;
        if (t[0] == "Sports") a = std::make_unique<SportsActivity>(t[1], t[2], t.size() > 3 ? t[3] : "");
        else                  a = std::make_unique<CulturalActivity>(t[1], t[2], t.size() > 3 ? t[3] : "");
        if (t.size() > 4 && !t[4].empty())
            for (auto& p : split(t[4], ',')) if (!p.empty()) a->enroll(p);
        activities.push_back(std::move(a));
    }

    std::ifstream fc(dir + "/appointments.txt");
    while (std::getline(fc, line)) {
        if (line.empty()) continue;
        auto t = split(line, '|');
        clinic.schedule(Appointment(t[0], t[1], t[2], t[3], t.size() > 4 ? t[4] : "Scheduled"));
    }
}

// ---------- Demo data (matches the dashboard mockup) ----------
void University::seedSampleData() {
    students.clear(); dormitories.clear(); bookings.clear();
    activities.clear(); clinic.getAppointments().clear();

    const char* ids = "ABCD";
    for (int i = 0; i < 4; ++i) {
        std::string id(1, ids[i]);
        Dormitory d(id, "Dormitory " + id);
        for (int n = 1; n <= 6; ++n) {
            int cap = (n % 3 == 0) ? 3 : (n % 2 == 0 ? 2 : 1);
            std::string num = id + "-" + std::to_string(100 * (i + 1) + n);
            d.addRoom(Room(num, cap, cap == 1 ? "Single" : cap == 2 ? "Double" : "Triple"));
        }
        d.getRestaurant().setMenu(Menu("Bread, Jam, Coffee", "Chicken Tajine, Salad", "Pasta, Yogurt, Fruit"));
        dormitories.push_back(d);
    }

    addStudent(Student("S001", "Ahmed Benali",   2));
    addStudent(Student("S002", "Sara Khelifi",   1));
    addStudent(Student("S003", "Karim Boudiaf",  3));
    addStudent(Student("S004", "Nour Hadj",      2));
    addStudent(Student("S005", "Yacine Brahim",  4));

    try {
        assignStudentToRoom("S001", "A", "A-101");
        assignStudentToRoom("S002", "A", "A-101");
        assignStudentToRoom("S003", "B", "B-202");
    } catch (...) {}

    addActivity(std::make_unique<SportsActivity>("Football Training", "Mon & Wed 18:00", "Coach Reda"));
    addActivity(std::make_unique<SportsActivity>("Basketball",        "Tue 17:00",       "Coach Samia"));
    addActivity(std::make_unique<CulturalActivity>("Film Evening",    "Sat 20:00",       "Auditorium"));
    addActivity(std::make_unique<CulturalActivity>("Book Club",       "Fri 16:00",       "Library"));

    clinic.schedule(Appointment("S004", "2026-06-08", "10:00", "General check-up"));
    bookMeal(MealBooking("S001", "2026-06-08", MealType::Dinner));
}
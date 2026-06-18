#include "University.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <ctime>

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
    
    ArchiveItem ai;
    ai.archiveId = "arch_" + std::to_string(std::time(nullptr)) + "_" + id;
    ai.type = "Student";
    ai.objectId = s->getId();
    ai.parentId = "";
    ai.name = s->getFullName();
    
    char buf[20];
    std::time_t now = std::time(nullptr);
    if (const std::tm* tm = std::localtime(&now))
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tm);
    ai.deletedAt = buf;
    
    ai.data = s->getId() + "|" + s->getFullName() + "|" + std::to_string(s->getAcademicYear()) + "|" + s->getDormitoryId() + "|" + s->getRoomNumber();
    archive.push_back(ai);

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

void University::reassignStudent(const std::string& studentId,
                                 const std::string& newDormId,
                                 const std::string& newRoomNumber) {
    Student* s = findStudent(studentId);
    if (!s) throw std::runtime_error("Student not found: " + studentId);

    // Save old placement for rollback
    std::string oldDorm = s->getDormitoryId();
    std::string oldRoom = s->getRoomNumber();

    // Remove from current room (no-op if unassigned)
    removeStudentFromRoom(studentId);

    // Assign to new room; rollback if it fails
    try {
        assignStudentToRoom(studentId, newDormId, newRoomNumber);
    } catch (...) {
        if (!oldRoom.empty()) {
            try { assignStudentToRoom(studentId, oldDorm, oldRoom); } catch (...) {}
        }
        throw;   // re-throw original error
    }
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
    }

    std::ofstream fm(dir + "/weekly_menu.txt");
    for (int i = 0; i < 7; ++i) {
        fm << weeklyMenu.days[i].breakfast << '|' << weeklyMenu.days[i].lunch << '|'
           << weeklyMenu.days[i].dinner << '|' << weeklyMenu.days[i].mealsServed << '\n';
    }

    std::ofstream fb(dir + "/bookings.txt");
    for (const auto& b : bookings)
        fb << b.studentId << '|' << b.date << '|' << mealTypeToString(b.meal)
           << '|' << (b.confirmed ? 1 : 0) << '|' << (b.served ? 1 : 0) << '\n';

    std::ofstream fa(dir + "/activities.txt");
    for (const auto& a : activities) {
        std::string extra;
        if (auto* sp = dynamic_cast<SportsActivity*>(a.get()))   extra = sp->getCoach();
        else if (auto* cu = dynamic_cast<CulturalActivity*>(a.get())) extra = cu->getVenue();
        fa << a->category() << '|' << a->getName() << '|' << a->getSchedule() << '|' << extra << '|';
        const auto& enr = a->getEnrollments();
        for (size_t i = 0; i < enr.size(); ++i) {
            fa << enr[i].studentId << ':' << enr[i].status;
            if (i + 1 < enr.size()) fa << ',';
        }
        fa << '\n';
    }

    std::ofstream fc(dir + "/appointments.txt");
    for (const auto& ap : clinic.getAppointments())
        fc << ap.studentId << '|' << ap.date << '|' << ap.time << '|'
           << ap.reason << '|' << ap.status << '\n';

    std::ofstream farch(dir + "/archive.txt");
    for (const auto& ai : archive) {
        farch << ai.archiveId << "||" << ai.type << "||" << ai.objectId << "||"
              << ai.parentId << "||" << ai.name << "||" << ai.deletedAt << "||" << ai.data << '\n';
    }

    // ── Auto-generate credentials.txt ────────────────────────────────
    std::ofstream cr(dir + "/credentials.txt");
    cr << "================================================================================\n";
    cr << "                    DORESM - User Credentials Reference\n";
    cr << "                    University Dormitory & Restaurant\n";
    cr << "                         Management System\n";
    cr << "================================================================================\n\n";
    cr << "This file is auto-generated each time the application saves data.\n";
    cr << "It lists all valid login credentials for every user role.\n\n";

    cr << std::string(80, '-') << "\n";
    cr << "  ROLE: ADMIN\n";
    cr << std::string(80, '-') << "\n";
    cr << "  Username:  admin\n";
    cr << "  Password:  admin\n\n";

    cr << std::string(80, '-') << "\n";
    cr << "  ROLE: STAFF (one per dormitory)\n";
    cr << "  Login with: Staff X (case-insensitive) / staffX  where X = dormitory ID\n";
    cr << std::string(80, '-') << "\n";
    for (const auto& d : dormitories) {
        std::string staffUser = "Staff " + d.getId();
        std::string staffPass = "staff" + d.getId();
        cr << "  Username:  " << staffUser;
        std::string pad(std::max(1, 25 - static_cast<int>(staffUser.size())), ' ');
        cr << pad << "Password:  " << staffPass << "\n";
    }
    cr << "\n";

    cr << std::string(80, '-') << "\n";
    cr << "  ROLE: DORMITORY ADMIN\n";
    cr << "  Login with: dormitory name (case-insensitive) / dormitory ID\n";
    cr << std::string(80, '-') << "\n";
    for (const auto& d : dormitories) {
        cr << "  Username:  " << d.getName();
        // Pad to align password column
        std::string pad(std::max(1, 25 - static_cast<int>(d.getName().size())), ' ');
        cr << pad << "Password:  " << d.getId() << "\n";
    }
    cr << "\n";

    cr << std::string(80, '-') << "\n";
    cr << "  ROLE: STUDENT\n";
    cr << "  Login with: full name (case-insensitive) / student ID\n";
    cr << std::string(80, '-') << "\n";
    for (const auto& s : students) {
        cr << "  Username:  " << s.getFullName();
        std::string pad(std::max(1, 25 - static_cast<int>(s.getFullName().size())), ' ');
        cr << pad << "Password:  " << s.getId() << "\n";
    }
    cr << "\n";

    cr << "================================================================================\n";
    cr << "  NOTE: This file is regenerated automatically when data is saved.\n";
    cr << "  New students and dormitories added through the app will appear here.\n";
    cr << "================================================================================\n";
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
            // Deprecated format, ignore
        }
    }

    std::ifstream fm(dir + "/weekly_menu.txt");
    int dIdx = 0;
    while (std::getline(fm, line) && dIdx < 7) {
        auto tokens = split(line, '|');
        if (tokens.size() >= 4) {
            weeklyMenu.days[dIdx].breakfast = tokens[0];
            weeklyMenu.days[dIdx].lunch = tokens[1];
            weeklyMenu.days[dIdx].dinner = tokens[2];
            weeklyMenu.days[dIdx].mealsServed = std::stoi(tokens[3]);
        }
        ++dIdx;
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
        bool conf = t.size() > 3 && t[3] == "1";
        bool serv = t.size() > 4 && t[4] == "1";
        bookings.emplace_back(t[0], t[1], mealTypeFromString(t[2]), conf, serv);
    }

    std::ifstream fa(dir + "/activities.txt");
    while (std::getline(fa, line)) {
        if (line.empty()) continue;
        auto t = split(line, '|');
        std::unique_ptr<Activity> a;
        if (t[0] == "Sports") a = std::make_unique<SportsActivity>(t[1], t[2], t.size() > 3 ? t[3] : "");
        else                  a = std::make_unique<CulturalActivity>(t[1], t[2], t.size() > 3 ? t[3] : "");
        if (t.size() > 4 && !t[4].empty()) {
            for (auto& entry : split(t[4], ',')) {
                if (entry.empty()) continue;
                auto colonPos = entry.find(':');
                if (colonPos != std::string::npos) {
                    // New format: studentId:status
                    std::string sid = entry.substr(0, colonPos);
                    std::string status = entry.substr(colonPos + 1);
                    a->getEnrollments().emplace_back(sid, status);
                } else {
                    // Legacy format: plain student ID → treat as Approved
                    a->getEnrollments().emplace_back(entry, "Approved");
                }
            }
            a->rebuildParticipants();
        }
        activities.push_back(std::move(a));
    }

    std::ifstream fc(dir + "/appointments.txt");
    while (std::getline(fc, line)) {
        if (line.empty()) continue;
        auto t = split(line, '|');
        clinic.schedule(Appointment(t[0], t[1], t[2], t[3], t.size() > 4 ? t[4] : "Scheduled"));
    }

    std::ifstream farch(dir + "/archive.txt");
    while (std::getline(farch, line)) {
        if (line.empty()) continue;
        size_t pos = 0;
        std::vector<std::string> parts;
        while ((pos = line.find("||")) != std::string::npos) {
            parts.push_back(line.substr(0, pos));
            line.erase(0, pos + 2);
        }
        parts.push_back(line);
        if (parts.size() >= 7) {
            ArchiveItem ai;
            ai.archiveId = parts[0];
            ai.type = parts[1];
            ai.objectId = parts[2];
            ai.parentId = parts[3];
            ai.name = parts[4];
            ai.deletedAt = parts[5];
            ai.data = parts[6];
            archive.push_back(ai);
        }
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
        dormitories.push_back(d);
    }

    for (int i = 0; i < 7; ++i) {
        weeklyMenu.days[i].breakfast = "Bread, Jam, Coffee";
        weeklyMenu.days[i].lunch = "Chicken Tajine, Salad";
        weeklyMenu.days[i].dinner = "Pasta, Yogurt, Fruit";
        weeklyMenu.days[i].mealsServed = 0;
    }

    addStudent(Student("001", "Ahmed Benali",   2));
    addStudent(Student("002", "Sara Khelifi",   1));
    addStudent(Student("003", "Karim Boudiaf",  3));
    addStudent(Student("004", "Nour Hadj",      2));
    addStudent(Student("005", "Yacine Brahim",  4));

    try {
        assignStudentToRoom("001", "A", "A-101");
        assignStudentToRoom("002", "A", "A-101");
        assignStudentToRoom("003", "B", "B-202");
    } catch (...) {}

    addActivity(std::make_unique<SportsActivity>("Football Training", "Mon & Wed 18:00", "Coach Reda"));
    addActivity(std::make_unique<SportsActivity>("Basketball",        "Tue 17:00",       "Coach Samia"));
    addActivity(std::make_unique<CulturalActivity>("Film Evening",    "Sat 20:00",       "Auditorium"));
    addActivity(std::make_unique<CulturalActivity>("Book Club",       "Fri 16:00",       "Library"));

    clinic.schedule(Appointment("004", "2026-06-08", "10:00", "General check-up"));
    bookMeal(MealBooking("001", "2026-06-08", MealType::Dinner));
}

// ---------- Activity log ----------
void University::logActivity(const std::string& emoji, const std::string& text) {
    std::time_t now = std::time(nullptr);
    char buf[16] = "";
    if (const std::tm* tm = std::localtime(&now))
        std::strftime(buf, sizeof(buf), "%H:%M", tm);
    activityLog.insert(activityLog.begin(), ActivityLogItem{emoji, text, buf}); // newest first
    if (activityLog.size() > 50) activityLog.resize(50);
}

// ---------- Dormitories ----------
void University::removeDormitory(const std::string& id) {
    Dormitory* d = findDormitory(id);
    if (!d) throw std::runtime_error("Dormitory not found: " + id);
    // free any students living in this dormitory so records stay consistent
    for (auto& s : students)
        if (s.isAccommodated() && s.getDormitoryId() == id)
            s.clearAccommodation();

    ArchiveItem ai;
    ai.archiveId = "arch_" + std::to_string(std::time(nullptr)) + "_" + id;
    ai.type = "Dormitory";
    ai.objectId = d->getId();
    ai.parentId = "";
    ai.name = d->getName();
    
    char buf[20];
    std::time_t now = std::time(nullptr);
    if (const std::tm* tm = std::localtime(&now))
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tm);
    ai.deletedAt = buf;
    
    std::stringstream ss;
    ss << "DORM|" << d->getId() << "|" << d->getName();
    for (const auto& r : d->getRooms()) {
        ss << "~ROOM|" << r.getNumber() << "|" << r.getCapacity() << "|" << r.getType() << "|" << (r.isUnderMaintenance() ? 1 : 0);
    }
    ai.data = ss.str();
    archive.push_back(ai);

    dormitories.erase(std::remove_if(dormitories.begin(), dormitories.end(),
        [&](const Dormitory& x){ return x.getId() == id; }), dormitories.end());
}

void University::removeRoomFromDormitory(const std::string& dormId, const std::string& roomNumber) {
    Dormitory* d = findDormitory(dormId);
    if (!d) throw std::runtime_error("Dormitory not found: " + dormId);
    Room* r = d->findRoom(roomNumber);
    if (!r) throw std::runtime_error("Room not found: " + roomNumber);
    if (!r->isEmpty()) throw std::runtime_error("Cannot delete a room that still has residents");
    
    ArchiveItem ai;
    ai.archiveId = "arch_" + std::to_string(std::time(nullptr)) + "_" + roomNumber;
    ai.type = "Room";
    ai.objectId = r->getNumber();
    ai.parentId = dormId;
    ai.name = "Room " + r->getNumber() + " (" + r->getType() + ")";
    
    char buf[20];
    std::time_t now = std::time(nullptr);
    if (const std::tm* tm = std::localtime(&now))
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tm);
    ai.deletedAt = buf;
    
    ai.data = r->getNumber() + "|" + std::to_string(r->getCapacity()) + "|" + r->getType() + "|" + (r->isUnderMaintenance() ? "1" : "0");
    archive.push_back(ai);
    
    d->removeRoom(roomNumber);
}

void University::restoreFromArchive(const std::string& archiveId) {
    auto it = std::find_if(archive.begin(), archive.end(), [&](const ArchiveItem& a){ return a.archiveId == archiveId; });
    if (it == archive.end()) throw std::runtime_error("Archive item not found");
    
    if (it->type == "Student") {
        auto parts = split(it->data, '|');
        if (parts.size() >= 3) {
            Student s(parts[0], parts[1], std::stoi(parts[2]));
            addStudent(s);
        }
    } else if (it->type == "Room") {
        auto parts = split(it->data, '|');
        if (parts.size() >= 4) {
            Dormitory* d = findDormitory(it->parentId);
            if (!d) throw std::runtime_error("Parent dormitory no longer exists. Restore it first.");
            Room r(parts[0], std::stoi(parts[1]), parts[2]);
            if (parts[3] == "1") r.setMaintenance(true);
            d->addRoom(r);
        }
    } else if (it->type == "Dormitory") {
        auto lines = split(it->data, '~');
        if (!lines.empty()) {
            auto dParts = split(lines[0], '|');
            if (dParts.size() >= 3 && dParts[0] == "DORM") {
                Dormitory d(dParts[1], dParts[2]);
                for (size_t i = 1; i < lines.size(); ++i) {
                    auto rParts = split(lines[i], '|');
                    if (rParts.size() >= 5 && rParts[0] == "ROOM") {
                        Room r(rParts[1], std::stoi(rParts[2]), rParts[3]);
                        if (rParts[4] == "1") r.setMaintenance(true);
                        d.addRoom(r);
                    }
                }
                addDormitory(d);
            }
        }
    }
    archive.erase(it);
}

void University::permanentlyDeleteArchive(const std::string& archiveId) {
    archive.erase(std::remove_if(archive.begin(), archive.end(), 
        [&](const ArchiveItem& a){ return a.archiveId == archiveId; }), archive.end());
}

void University::emptyArchive() {
    archive.clear();
}

// ---------- Time-based cleanup ----------
void University::cleanupExpired() {
    // Get today's date as YYYY-MM-DD string
    std::time_t now = std::time(nullptr);
    char today[16] = "";
    char nowTime[16] = "";
    if (const std::tm* tm = std::localtime(&now)) {
        std::strftime(today, sizeof(today), "%Y-%m-%d", tm);
        std::strftime(nowTime, sizeof(nowTime), "%H:%M", tm);
    }
    std::string todayStr(today);
    std::string nowTimeStr(nowTime);

    // Remove meal bookings whose date has passed (strictly before today)
    bookings.erase(
        std::remove_if(bookings.begin(), bookings.end(),
            [&](const MealBooking& b) { return b.date < todayStr; }),
        bookings.end());

    // Remove appointments whose date has passed
    auto& apts = clinic.getAppointments();
    apts.erase(
        std::remove_if(apts.begin(), apts.end(),
            [&](const Appointment& a) {
                // Past date → remove
                if (a.date < todayStr) return true;
                // Same date but time has passed → remove
                if (a.date == todayStr && a.time < nowTimeStr) return true;
                return false;
            }),
        apts.end());
}
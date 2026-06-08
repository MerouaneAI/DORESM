#include "Dormitory.h"
#include <stdexcept>
#include <algorithm>

Dormitory::Dormitory(std::string id, std::string name)
    : id(std::move(id)), name(std::move(name)),
      restaurant(this->name + " Restaurant") {}

void Dormitory::addRoom(const Room& r) {
    if (findRoom(r.getNumber()))
        throw std::runtime_error("Room already exists: " + r.getNumber());
    rooms.push_back(r);
}

Room* Dormitory::findRoom(const std::string& number) {
    for (auto& r : rooms)
        if (r.getNumber() == number) return &r;
    return nullptr;
}

void Dormitory::removeRoom(const std::string& number) {
    Room* r = findRoom(number);
    if (!r) throw std::runtime_error("Room not found: " + number);
    if (!r->isEmpty())
        throw std::runtime_error("Cannot delete a room that still has residents");
    rooms.erase(std::remove_if(rooms.begin(), rooms.end(),
        [&](const Room& x){ return x.getNumber() == number; }), rooms.end());
}

int Dormitory::totalCapacity() const {
    int t = 0; for (const auto& r : rooms) t += r.getCapacity(); return t;
}

int Dormitory::totalOccupancy() const {
    int t = 0; for (const auto& r : rooms) t += r.getOccupancy(); return t;
}

int Dormitory::availableRoomCount() const {
    int t = 0;
    for (const auto& r : rooms)
        if (!r.isFull() && !r.isUnderMaintenance()) ++t;
    return t;
}

void Dormitory::assignStudent(const std::string& roomNumber, const std::string& studentId) {
    Room* r = findRoom(roomNumber);
    if (!r) throw std::runtime_error("Room not found: " + roomNumber);
    r->addOccupant(studentId);   // enforces capacity / maintenance
}

void Dormitory::removeStudent(const std::string& roomNumber, const std::string& studentId) {
    Room* r = findRoom(roomNumber);
    if (!r) throw std::runtime_error("Room not found: " + roomNumber);
    r->removeOccupant(studentId);
}
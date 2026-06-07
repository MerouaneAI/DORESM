#include "Room.h"
#include <algorithm>
#include <stdexcept>

Room::Room(std::string number, int capacity, std::string type)
    : number(std::move(number)), capacity(capacity), type(std::move(type)) {
    if (this->capacity <= 0)
        throw std::invalid_argument("Room capacity must be positive");
}

bool Room::hasOccupant(const std::string& studentId) const {
    return std::find(occupants.begin(), occupants.end(), studentId) != occupants.end();
}

void Room::addOccupant(const std::string& studentId) {
    if (maintenance)            throw std::runtime_error("Room " + number + " is under maintenance");
    if (isFull())               throw std::runtime_error("Room " + number + " is full");
    if (hasOccupant(studentId)) throw std::runtime_error("Student already in room " + number);
    occupants.push_back(studentId);
}

void Room::removeOccupant(const std::string& studentId) {
    auto it = std::find(occupants.begin(), occupants.end(), studentId);
    if (it == occupants.end())
        throw std::runtime_error("Student not found in room " + number);
    occupants.erase(it);
}

std::string Room::status() const {
    if (maintenance) return "Maintenance";
    if (isFull())    return "Full";
    if (isEmpty())   return "Available";
    return "Partial";
}
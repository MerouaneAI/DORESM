#pragma once
#include <string>
#include <vector>
#include "Room.h"
#include "Restaurant.h"

// COMPOSITION: a Dormitory owns its Rooms and exactly one Restaurant.
class Dormitory {
private:
    std::string id;            // e.g. "A"
    std::string name;          // e.g. "Dormitory A"
    std::vector<Room> rooms;   // owned
    Restaurant restaurant;     // owned (exactly one)
public:
    Dormitory() = default;
    Dormitory(std::string id, std::string name);

    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }

    Restaurant&       getRestaurant()       { return restaurant; }
    const Restaurant& getRestaurant() const { return restaurant; }

    std::vector<Room>&       getRooms()       { return rooms; }
    const std::vector<Room>& getRooms() const { return rooms; }

    void  addRoom(const Room& r);
    Room* findRoom(const std::string& number);   // nullptr if not found

    int totalCapacity() const;
    int totalOccupancy() const;
    int availableRoomCount() const;

    void assignStudent(const std::string& roomNumber, const std::string& studentId);
    void removeStudent(const std::string& roomNumber, const std::string& studentId);
};
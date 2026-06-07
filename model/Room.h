#pragma once
#include <string>
#include <vector>

class Room {
private:
    std::string number;
    int capacity = 0;
    std::string type;                   // Single / Double / Triple
    bool maintenance = false;
    std::vector<std::string> occupants; // student IDs
public:
    Room() = default;
    Room(std::string number, int capacity, std::string type);

    const std::string& getNumber() const { return number; }
    int  getCapacity() const { return capacity; }
    const std::string& getType() const { return type; }
    bool isUnderMaintenance() const { return maintenance; }
    void setMaintenance(bool m) { maintenance = m; }

    const std::vector<std::string>& getOccupants() const { return occupants; }
    int  getOccupancy() const { return static_cast<int>(occupants.size()); }
    bool isFull()  const { return getOccupancy() >= capacity; }
    bool isEmpty() const { return occupants.empty(); }
    bool hasOccupant(const std::string& studentId) const;

    void addOccupant(const std::string& studentId);     // throws on full / maintenance / duplicate
    void removeOccupant(const std::string& studentId);  // throws if not present

    std::string status() const;         // Full / Partial / Available / Maintenance
};
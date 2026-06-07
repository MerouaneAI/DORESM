#pragma once
#include <string>
#include <utility>

class Person {
protected:
    std::string id;
    std::string fullName;
public:
    Person() = default;
    Person(std::string id, std::string fullName)
        : id(std::move(id)), fullName(std::move(fullName)) {}
    virtual ~Person() = default;

    const std::string& getId() const { return id; }
    const std::string& getFullName() const { return fullName; }
    void setFullName(const std::string& name) { fullName = name; }

    virtual std::string role() const = 0;   // polymorphic
};
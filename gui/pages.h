#pragma once
#include <QWidget>
#include <QString>
class University;
class QVBoxLayout;

class Refreshable {
public:
    virtual void refresh() = 0;
    virtual ~Refreshable() = default;
};

class DashboardPage : public QWidget, public Refreshable {
public:
    explicit DashboardPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root;
};

class DormitoriesPage : public QWidget, public Refreshable {
public:
    explicit DormitoriesPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root;
};

class RoomsPage : public QWidget, public Refreshable {
public:
    explicit RoomsPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root; void assignDialog();
};

class StudentsPage : public QWidget, public Refreshable {
public:
    explicit StudentsPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root; void addDialog();
};

class RestaurantPage : public QWidget, public Refreshable {
public:
    explicit RestaurantPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root;
};

class MealBookingPage : public QWidget, public Refreshable {
public:
    explicit MealBookingPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root; void bookDialog();
};

class HealthPage : public QWidget, public Refreshable {
public:
    explicit HealthPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root; void scheduleDialog();
};

class ActivityPage : public QWidget, public Refreshable {
public:
    ActivityPage(University& uni, QString category, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QString category; QVBoxLayout* root;
    void addDialog(); void enrollDialog();
};
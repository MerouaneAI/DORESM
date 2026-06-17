#pragma once
#include <QWidget>
#include <string>

class University;
class QVBoxLayout;

// Interface for refreshable staff pages
class StaffRefreshable {
public:
    virtual void refresh() = 0;
    virtual ~StaffRefreshable() = default;
};

// Dashboard — small overview with maintenance + meal stats (scoped to one dormitory)
class StaffDashboardPage : public QWidget, public StaffRefreshable {
    Q_OBJECT
public:
    explicit StaffDashboardPage(University& uni, const std::string& dormId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    std::string dormId;
    QVBoxLayout* root;
};

// Maintenance — rooms under maintenance with "Mark Fixed" action (scoped to one dormitory)
class StaffMaintenancePage : public QWidget, public StaffRefreshable {
    Q_OBJECT
public:
    explicit StaffMaintenancePage(University& uni, const std::string& dormId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    std::string dormId;
    QVBoxLayout* root;
};

// Restaurant — today's menu + serve meals for bookings (scoped to one dormitory)
class StaffRestaurantPage : public QWidget, public StaffRefreshable {
    Q_OBJECT
public:
    explicit StaffRestaurantPage(University& uni, const std::string& dormId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    std::string dormId;
    QVBoxLayout* root;
};

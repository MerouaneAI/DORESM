#pragma once
#include <QWidget>

class University;
class QVBoxLayout;

// Interface for refreshable staff pages
class StaffRefreshable {
public:
    virtual void refresh() = 0;
    virtual ~StaffRefreshable() = default;
};

// Dashboard — small overview with maintenance + meal stats
class StaffDashboardPage : public QWidget, public StaffRefreshable {
    Q_OBJECT
public:
    explicit StaffDashboardPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    QVBoxLayout* root;
};

// Maintenance — rooms under maintenance with "Mark Fixed" action
class StaffMaintenancePage : public QWidget, public StaffRefreshable {
    Q_OBJECT
public:
    explicit StaffMaintenancePage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    QVBoxLayout* root;
};

// Restaurant — today's menu + serve meals for bookings
class StaffRestaurantPage : public QWidget, public StaffRefreshable {
    Q_OBJECT
public:
    explicit StaffRestaurantPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    QVBoxLayout* root;
};

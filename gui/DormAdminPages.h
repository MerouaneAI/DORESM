#pragma once
#include <QWidget>
#include <string>
#include <functional>

class University;
class QVBoxLayout;
class QLineEdit;
class QComboBox;
class QTableWidget;

// Interface for refreshable dorm-admin pages
class DormAdminRefreshable {
public:
    virtual void refresh() = 0;
    virtual ~DormAdminRefreshable() = default;
};

// ── Dashboard ────────────────────────────────────────────────────────────────
class DormAdminDashboardPage : public QWidget, public DormAdminRefreshable {
    Q_OBJECT
public:
    DormAdminDashboardPage(University& uni, const std::string& dormId, QWidget* parent = nullptr);
    void setNavigator(std::function<void(int)> nav);
    void refresh() override;
private:
    University& uni;
    std::string dormId;
    QVBoxLayout* root;
    std::function<void(int)> navigate;
};

// ── Dormitory (single-dorm view with room add/delete) ────────────────────────
class DormAdminDormitoryPage : public QWidget, public DormAdminRefreshable {
    Q_OBJECT
public:
    DormAdminDormitoryPage(University& uni, const std::string& dormId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    std::string dormId;
    QVBoxLayout* root;
    void addRoomDialog();
    void deleteRoomDialog();
};

// ── Rooms (full management, scoped to this dorm) ─────────────────────────────
class DormAdminRoomsPage : public QWidget, public DormAdminRefreshable {
    Q_OBJECT
public:
    DormAdminRoomsPage(University& uni, const std::string& dormId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    std::string dormId;
    QVBoxLayout* root;
    std::string fStudentId, fRoom;
    int fTypeIdx = 0, fStatusIdx = 0;
    void assignDialog();
    void reassignDialog();
    void unassignDialog();
    void toggleMaintenanceDialog(QTableWidget* table);
    void applyFilters(QTableWidget* table, QComboBox* typeBox, QComboBox* statusBox,
                      QLineEdit* studentEdit, QLineEdit* roomEdit);
};

// ── Students (read-only, filtered to this dorm) ──────────────────────────────
class DormAdminStudentsPage : public QWidget, public DormAdminRefreshable {
    Q_OBJECT
public:
    DormAdminStudentsPage(University& uni, const std::string& dormId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    std::string dormId;
    QVBoxLayout* root;
    std::string fStudentId, fName;
    int fYearIdx = 0;
    void applyFilters(QTableWidget* table, QLineEdit* idEdit, QLineEdit* nameEdit, QComboBox* yearBox);
};

// ── Restaurant (read-only weekly menu) ───────────────────────────────────────
class DormAdminRestaurantPage : public QWidget, public DormAdminRefreshable {
    Q_OBJECT
public:
    DormAdminRestaurantPage(University& uni, const std::string& dormId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    std::string dormId;
    QVBoxLayout* root;
};

// ── Meal Booking (read-only, filtered to this dorm's students) ───────────────
class DormAdminMealBookingPage : public QWidget, public DormAdminRefreshable {
    Q_OBJECT
public:
    DormAdminMealBookingPage(University& uni, const std::string& dormId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    std::string dormId;
    QVBoxLayout* root;
};

// ── Activities (read-only, Sports or Cultural) ───────────────────────────────
class DormAdminActivityPage : public QWidget, public DormAdminRefreshable {
    Q_OBJECT
public:
    DormAdminActivityPage(University& uni, const std::string& dormId,
                          const std::string& category, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    std::string dormId;
    std::string category;
    QVBoxLayout* root;
};

// ── Health Clinic (read-only, filtered to this dorm's students) ──────────────
class DormAdminHealthPage : public QWidget, public DormAdminRefreshable {
    Q_OBJECT
public:
    DormAdminHealthPage(University& uni, const std::string& dormId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    std::string dormId;
    QVBoxLayout* root;
};

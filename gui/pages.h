#pragma once
#include <QWidget>
#include <QString>
#include <functional>
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
    void setNavigator(std::function<void(int)> nav);   // NEW
    void refresh() override;
private:
    University& uni; QVBoxLayout* root;
    std::function<void(int)> navigate;                 // NEW
};

class DormitoriesPage : public QWidget, public Refreshable {
public:
    explicit DormitoriesPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root;
    void addDormitoryDialog();
    void deleteDormitoryDialog(const QString& dormId);
    void addRoomDialog(const QString& dormId);
    void deleteRoomDialog(const QString& dormId);
};

class QLineEdit;
class QComboBox;
class QTableWidget;

class RoomsPage : public QWidget, public Refreshable {
public:
    explicit RoomsPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root;
    // Filter widgets (persisted across refreshes by storing filter values)
    QString fStudentId, fRoom;
    int fDormIdx = 0, fTypeIdx = 0, fStatusIdx = 0;
    void assignDialog();
    void reassignDialog();
    void unassignDialog();
    void toggleMaintenanceDialog(QTableWidget* table);
    void applyFilters(QTableWidget* table, QComboBox* dormBox,
                      QComboBox* typeBox, QComboBox* statusBox,
                      QLineEdit* studentEdit, QLineEdit* roomEdit);
};

class StudentsPage : public QWidget, public Refreshable {
public:
    explicit StudentsPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root;
    // Filter widgets (persisted across refreshes)
    QString fStudentId, fName;
    int fDormIdx = 0;
    int fYearIdx = 0;
    
    void addDialog();
    void editDialog();
    void applyFilters(QTableWidget* table, QLineEdit* idEdit, QLineEdit* nameEdit, 
                      QComboBox* dormBox, QComboBox* yearBox);
};

class RestaurantPage : public QWidget, public Refreshable {
public:
    explicit RestaurantPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root;
    void saveMenu(QTableWidget* table);
};

class MealBookingPage : public QWidget, public Refreshable {
public:
    explicit MealBookingPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root;
    void bookDialog();
};

class HealthPage : public QWidget, public Refreshable {
public:
    explicit HealthPage(University& uni, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QVBoxLayout* root;
    void scheduleDialog();
};

class ActivityPage : public QWidget, public Refreshable {
public:
    ActivityPage(University& uni, QString category, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni; QString category; QVBoxLayout* root;
    void addDialog(); void enrollDialog();
};
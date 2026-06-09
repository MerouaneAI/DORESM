#pragma once
#include <QWidget>
#include <QString>

class University;
class QVBoxLayout;

// Interface for refreshable student pages
class StudentRefreshable {
public:
    virtual void refresh() = 0;
    virtual ~StudentRefreshable() = default;
};

// Student Dashboard — overview of accommodation, menu, events
class StudentDashboardPage : public QWidget, public StudentRefreshable {
    Q_OBJECT
public:
    explicit StudentDashboardPage(University& uni, const QString& studentId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    QString studentId;
    QVBoxLayout* root;
};

// Events page — sports + cultural events, with apply action
class StudentEventsPage : public QWidget, public StudentRefreshable {
    Q_OBJECT
public:
    explicit StudentEventsPage(University& uni, const QString& studentId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    QString studentId;
    QVBoxLayout* root;
};

// Health page — book appointment with a doctor
class StudentHealthPage : public QWidget, public StudentRefreshable {
    Q_OBJECT
public:
    explicit StudentHealthPage(University& uni, const QString& studentId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    QString studentId;
    QVBoxLayout* root;
    void bookAppointmentDialog();
};

// Meal Booking page — book meals directly
class StudentMealBookingPage : public QWidget, public StudentRefreshable {
    Q_OBJECT
public:
    explicit StudentMealBookingPage(University& uni, const QString& studentId, QWidget* parent = nullptr);
    void refresh() override;
private:
    University& uni;
    QString studentId;
    QVBoxLayout* root;
    void bookMealDialog();
};

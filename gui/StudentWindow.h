#pragma once
#include <QMainWindow>
#include <QList>
#include <QString>

class University;
class Student;
class QStackedWidget;
class QPushButton;
class QVBoxLayout;

class StudentWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit StudentWindow(University& uni, const QString& studentId, QWidget* parent = nullptr);
signals:
    void loggedOut();
private:
    University& uni;
    QString studentId;
    QStackedWidget* stack = nullptr;
    QList<QPushButton*> navButtons;

    void addNav(QVBoxLayout* sideLay, const QString& icon,
                const QString& label, int index);
    void switchPage(int index);
    Student* currentStudent();
};

#pragma once
#include <QMainWindow>
#include <QList>
#include <string>

class University;
class QStackedWidget;
class QPushButton;
class QVBoxLayout;

class StaffWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit StaffWindow(University& uni, const std::string& dormId, QWidget* parent = nullptr);
signals:
    void loggedOut();
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    University& uni;
    std::string dormId;
    QStackedWidget* stack = nullptr;
    QList<QPushButton*> navButtons;
    void addNav(QVBoxLayout* sideLay, const QString& icon,
                const QString& label, int index);
    void switchPage(int index);
};

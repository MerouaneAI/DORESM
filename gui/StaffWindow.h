#pragma once
#include <QMainWindow>
#include <QList>

class University;
class QStackedWidget;
class QPushButton;
class QVBoxLayout;

class StaffWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit StaffWindow(University& uni, QWidget* parent = nullptr);
signals:
    void loggedOut();
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    University& uni;
    QStackedWidget* stack = nullptr;
    QList<QPushButton*> navButtons;
    void addNav(QVBoxLayout* sideLay, const QString& icon,
                const QString& label, int index);
    void switchPage(int index);
};

#pragma once
#include <QMainWindow>
#include <QList>
#include <vector>

class University;
class QStackedWidget;
class QPushButton;
class QVBoxLayout;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(University& uni, QWidget* parent = nullptr);
protected:
    void closeEvent(QCloseEvent* event) override;   // auto-save on exit
private:
    University& uni;
    QStackedWidget* stack = nullptr;
    QList<QPushButton*> navButtons;
    void addNav(QVBoxLayout* sideLay, const QString& label, int index);
    void switchPage(int index);
};
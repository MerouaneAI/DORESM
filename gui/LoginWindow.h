#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class University;

class LoginWindow : public QWidget {
    Q_OBJECT
public:
    explicit LoginWindow(University& uni, QWidget* parent = nullptr);
signals:
    void adminLoggedIn();
    void studentLoggedIn(const QString& studentId);
    void dormAdminLoggedIn(const QString& dormId);
    void staffLoggedIn(const QString& dormId);
private:
    University& uni;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QLabel*    errorLabel;
    void attemptLogin();
};

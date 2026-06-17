#include <QApplication>
#include <filesystem>
#include "style.h"
#include "model/University.h"
#include "gui/LoginWindow.h"
#include "gui/MainWindow.h"
#include "gui/StudentWindow.h"
#include "gui/DormAdminWindow.h"
#include "gui/StaffWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyleSheet(APP_STYLESHEET);

    University uni("University");
    if (std::filesystem::exists("data/dormitories.txt"))
        uni.loadFromFiles("data");
    else {
        uni.seedSampleData();
        uni.saveToFiles("data");   // generate data files including credentials.txt
    }

    // Clean up expired meal bookings and past appointments
    uni.cleanupExpired();

    LoginWindow*      loginWin      = nullptr;
    MainWindow*       adminWin      = nullptr;
    StudentWindow*    studentWin    = nullptr;
    DormAdminWindow*  dormAdminWin  = nullptr;
    StaffWindow*      staffWin      = nullptr;

    std::function<void()> showLogin;

    showLogin = [&]() {
        if (adminWin)      { adminWin->close();      delete adminWin;      adminWin = nullptr; }
        if (studentWin)    { studentWin->close();     delete studentWin;    studentWin = nullptr; }
        if (dormAdminWin)  { dormAdminWin->close();   delete dormAdminWin;  dormAdminWin = nullptr; }
        if (staffWin)      { staffWin->close();       delete staffWin;      staffWin = nullptr; }
        if (loginWin)      { loginWin->close();       delete loginWin;      loginWin = nullptr; }

        loginWin = new LoginWindow(uni);
        loginWin->resize(900, 600);

        QObject::connect(loginWin, &LoginWindow::adminLoggedIn, [&]() {
            loginWin->hide();
            adminWin = new MainWindow(uni);
            adminWin->resize(1200, 760);
            adminWin->show();

            QObject::connect(adminWin, &MainWindow::loggedOut, [&]() {
                uni.saveToFiles("data");
                showLogin();
            });
        });

        QObject::connect(loginWin, &LoginWindow::studentLoggedIn, [&](const QString& studentId) {
            loginWin->hide();
            studentWin = new StudentWindow(uni, studentId);
            studentWin->resize(1200, 760);
            studentWin->show();

            QObject::connect(studentWin, &StudentWindow::loggedOut, [&]() {
                uni.saveToFiles("data");
                showLogin();
            });
        });

        QObject::connect(loginWin, &LoginWindow::dormAdminLoggedIn, [&](const QString& dormId) {
            loginWin->hide();
            dormAdminWin = new DormAdminWindow(uni, dormId.toStdString());
            dormAdminWin->resize(1200, 760);
            dormAdminWin->show();

            QObject::connect(dormAdminWin, &DormAdminWindow::loggedOut, [&]() {
                uni.saveToFiles("data");
                showLogin();
            });
        });

        QObject::connect(loginWin, &LoginWindow::staffLoggedIn, [&](const QString& dormId) {
            loginWin->hide();
            staffWin = new StaffWindow(uni, dormId.toStdString());
            staffWin->resize(1100, 700);
            staffWin->show();

            QObject::connect(staffWin, &StaffWindow::loggedOut, [&]() {
                uni.saveToFiles("data");
                showLogin();
            });
        });

        loginWin->show();
    };

    showLogin();
    return app.exec();
}
#include <QApplication>
#include <filesystem>
#include "style.h"
#include "model/University.h"
#include "gui/LoginWindow.h"
#include "gui/MainWindow.h"
#include "gui/StudentWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyleSheet(APP_STYLESHEET);

    University uni("ENSIA");
    if (std::filesystem::exists("data/dormitories.txt"))
        uni.loadFromFiles("data");
    else
        uni.seedSampleData();

    LoginWindow*   loginWin   = nullptr;
    MainWindow*    adminWin   = nullptr;
    StudentWindow* studentWin = nullptr;

    std::function<void()> showLogin;

    showLogin = [&]() {
        if (adminWin)   { adminWin->close();   delete adminWin;   adminWin = nullptr; }
        if (studentWin) { studentWin->close();  delete studentWin; studentWin = nullptr; }
        if (loginWin)   { loginWin->close();    delete loginWin;   loginWin = nullptr; }

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

        loginWin->show();
    };

    showLogin();
    return app.exec();
}
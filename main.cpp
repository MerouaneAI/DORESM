#include <QApplication>
#include <filesystem>
#include "style.h"
#include "model/University.h"
#include "gui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyleSheet(APP_STYLESHEET);

    University uni("ENSIA");
    if (std::filesystem::exists("data/dormitories.txt"))
        uni.loadFromFiles("data");   // restore previous session
    else
        uni.seedSampleData();        // first run: demo data matching the mockups

    MainWindow w(uni);
    w.resize(1200, 760);
    w.show();
    return app.exec();
}
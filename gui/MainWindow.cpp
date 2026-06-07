#include "gui/MainWindow.h"
#include "gui/pages.h"
#include "model/University.h"

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QCloseEvent>

MainWindow::MainWindow(University& u, QWidget* parent) : QMainWindow(parent), uni(u) {
    setWindowTitle("UDRMS - University Dormitory & Restaurant Management System");

    auto* central = new QWidget;
    auto* h = new QHBoxLayout(central);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);

    // ---- Sidebar ----
    auto* sidebar = new QWidget; sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(240);
    auto* sideLay = new QVBoxLayout(sidebar);
    sideLay->setContentsMargins(16, 20, 16, 20);
    sideLay->setSpacing(6);

    auto* brand = new QLabel("\xF0\x9F\x8F\x9B UDRMS"); brand->setObjectName("Brand");
    auto* sub   = new QLabel("ENSIA Campus");          sub->setObjectName("BrandSub");
    sideLay->addWidget(brand);
    sideLay->addWidget(sub);
    sideLay->addSpacing(16);

    // ---- Pages (order must match nav) ----
    stack = new QStackedWidget;
    stack->addWidget(new DashboardPage(uni));                  // 0
    stack->addWidget(new DormitoriesPage(uni));                // 1
    stack->addWidget(new RoomsPage(uni));                      // 2
    stack->addWidget(new StudentsPage(uni));                   // 3
    stack->addWidget(new RestaurantPage(uni));                 // 4
    stack->addWidget(new MealBookingPage(uni));                // 5
    stack->addWidget(new ActivityPage(uni, "Sports"));         // 6
    stack->addWidget(new HealthPage(uni));                     // 7
    stack->addWidget(new ActivityPage(uni, "Cultural"));       // 8

    const char* labels[] = {
        "\xF0\x9F\x93\x8A  Dashboard",
        "\xF0\x9F\x8F\xA2  Dormitories",
        "\xF0\x9F\x9B\x8F  Rooms",
        "\xF0\x9F\x91\xA5  Students",
        "\xF0\x9F\x8D\xBD  Restaurant",
        "\xF0\x9F\x93\x85  Meal Booking",
        "\xE2\x9A\xBD  Sports Activities",
        "\xF0\x9F\x8F\xA5  Health Clinic",
        "\xF0\x9F\x8E\xAD  Cultural Activities"
    };
    for (int i = 0; i < 9; ++i) addNav(sideLay, labels[i], i);

    sideLay->addStretch();
    auto* admin = new QLabel("\xF0\x9F\x91\xA4  Admin");
    admin->setStyleSheet("color:#6B7280; padding:8px 14px;");
    sideLay->addWidget(admin);

    // ---- Content (scrollable) ----
    auto* scroll = new QScrollArea;
    scroll->setObjectName("Content");
    scroll->setWidgetResizable(true);
    scroll->setWidget(stack);

    h->addWidget(sidebar);
    h->addWidget(scroll, 1);
    setCentralWidget(central);

    if (!navButtons.isEmpty()) navButtons[0]->setChecked(true);
    switchPage(0);
}

void MainWindow::addNav(QVBoxLayout* sideLay, const QString& label, int index) {
    auto* btn = new QPushButton(label);
    btn->setObjectName("NavBtn");
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    connect(btn, &QPushButton::clicked, this, [this, index]{ switchPage(index); });
    sideLay->addWidget(btn);
    navButtons.append(btn);
}

void MainWindow::switchPage(int index) {
    for (int i = 0; i < navButtons.size(); ++i)
        navButtons[i]->setChecked(i == index);
    stack->setCurrentIndex(index);
    if (auto* r = dynamic_cast<Refreshable*>(stack->widget(index)))
        r->refresh();   // rebuild from live data each time the page is shown
}

void MainWindow::closeEvent(QCloseEvent* event) {
    uni.saveToFiles("data");   // persist on exit
    event->accept();
}
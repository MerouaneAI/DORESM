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
#include <QFrame>

MainWindow::MainWindow(University& u, QWidget* parent)
    : QMainWindow(parent), uni(u)
{
    setWindowTitle("UDRMS – University Dormitory & Restaurant Management System");

    auto* central = new QWidget;
    auto* h = new QHBoxLayout(central);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);

    // ── Sidebar ──────────────────────────────────────────────────────
    auto* sidebar = new QWidget;
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(220);

    auto* sideLay = new QVBoxLayout(sidebar);
    sideLay->setContentsMargins(12, 20, 12, 16);
    sideLay->setSpacing(2);

    // Brand
    auto* brandRow = new QHBoxLayout;
    auto* brandIcon = new QLabel("🏛");
    brandIcon->setFixedSize(36, 36);
    brandIcon->setAlignment(Qt::AlignCenter);
    brandIcon->setStyleSheet(
        "background:#4C6FFF; border-radius:10px; font-size:18px;");
    auto* brandCol = new QWidget;
    auto* brandColV = new QVBoxLayout(brandCol);
    brandColV->setContentsMargins(0, 0, 0, 0); brandColV->setSpacing(0);
    auto* brandName = new QLabel("UDRMS");
    brandName->setObjectName("Brand");
    auto* brandSub  = new QLabel("ENSIA 2025–26");
    brandSub->setObjectName("BrandSub");
    brandColV->addWidget(brandName);
    brandColV->addWidget(brandSub);
    brandRow->addWidget(brandIcon);
    brandRow->addWidget(brandCol, 1);
    sideLay->addLayout(brandRow);
    sideLay->addSpacing(20);

    // Section label
    auto* menuLbl = new QLabel("MAIN MENU");
    menuLbl->setObjectName("SideMenuLabel");
    sideLay->addWidget(menuLbl);

    // Pages
    stack = new QStackedWidget;
    auto* dashboard = new DashboardPage(uni);
    dashboard->setNavigator([this](int index){ switchPage(index); });
    stack->addWidget(dashboard);                  // 0
    stack->addWidget(new DormitoriesPage(uni));                // 1
    stack->addWidget(new RoomsPage(uni));                      // 2
    stack->addWidget(new StudentsPage(uni));                   // 3
    stack->addWidget(new RestaurantPage(uni));                 // 4
    stack->addWidget(new MealBookingPage(uni));                // 5
    stack->addWidget(new ActivityPage(uni, "Sports"));         // 6
    stack->addWidget(new HealthPage(uni));                     // 7
    stack->addWidget(new ActivityPage(uni, "Cultural"));       // 8

    struct NavItem { const char* icon; const char* label; };
    NavItem items[] = {
        {"📊", "Dashboard"},
        {"🏢", "Dormitories"},
        {"🛏", "Rooms"},
        {"👥", "Students"},
        {"🍽", "Restaurant"},
        {"📅", "Meal Booking"},
        {"⚽", "Sports Activities"},
        {"🏥", "Health Clinic"},
        {"🎭", "Cultural Activities"},
    };
    for (int i = 0; i < 9; ++i)
        addNav(sideLay, items[i].icon, items[i].label, i);

    sideLay->addStretch();

    // Admin footer
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: rgba(255,255,255,0.08);");
    sideLay->addWidget(sep);
    sideLay->addSpacing(8);

    auto* adminRow = new QHBoxLayout;
    auto* adminAv  = new QLabel("👤");
    adminAv->setFixedSize(34, 34);
    adminAv->setAlignment(Qt::AlignCenter);
    adminAv->setStyleSheet(
        "background:#2D3154; border-radius:10px; font-size:16px;");
    auto* adminCol  = new QWidget;
    auto* adminColV = new QVBoxLayout(adminCol);
    adminColV->setContentsMargins(0, 0, 0, 0); adminColV->setSpacing(1);
    auto* adminName = new QLabel("Admin User"); adminName->setObjectName("SideAdminName");
    auto* adminRole = new QLabel("System Admin"); adminRole->setObjectName("SideAdminRole");
    adminColV->addWidget(adminName);
    adminColV->addWidget(adminRole);
    adminRow->addWidget(adminAv);
    adminRow->addWidget(adminCol, 1);
    sideLay->addLayout(adminRow);

    // ── Content (scrollable) ─────────────────────────────────────────
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

void MainWindow::addNav(QVBoxLayout* sideLay, const QString& icon,
                        const QString& label, int index)
{
    auto* btn = new QPushButton("  " + icon + "   " + label);
    btn->setObjectName("NavBtn");
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(38);
    connect(btn, &QPushButton::clicked, this, [this, index]{ switchPage(index); });
    sideLay->addWidget(btn);
    navButtons.append(btn);
}

void MainWindow::switchPage(int index) {
    for (int i = 0; i < navButtons.size(); ++i)
        navButtons[i]->setChecked(i == index);
    stack->setCurrentIndex(index);
    if (auto* r = dynamic_cast<Refreshable*>(stack->widget(index)))
        r->refresh();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    uni.saveToFiles("data");
    event->accept();
}
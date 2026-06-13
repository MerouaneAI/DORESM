#include "gui/DormAdminWindow.h"
#include "gui/DormAdminPages.h"
#include "model/University.h"

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>
#include <QCloseEvent>

DormAdminWindow::DormAdminWindow(University& u, const std::string& did,
                                 QWidget* parent)
    : QMainWindow(parent), uni(u), dormId(did)
{
    Dormitory* d = uni.findDormitory(dormId);
    QString dormName = d ? QString::fromStdString(d->getName()) : "Dormitory";

    setWindowTitle("UDRMS – Dorm Admin • " + dormName);

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
    auto* brandIcon = new QLabel("🏢");
    brandIcon->setFixedSize(36, 36);
    brandIcon->setAlignment(Qt::AlignCenter);
    brandIcon->setStyleSheet(
        "background:#4C6FFF; border-radius:10px; font-size:18px;");
    auto* brandCol = new QWidget;
    auto* brandColV = new QVBoxLayout(brandCol);
    brandColV->setContentsMargins(0, 0, 0, 0); brandColV->setSpacing(0);
    auto* brandName = new QLabel("UDRMS");
    brandName->setObjectName("Brand");
    auto* brandSub  = new QLabel("Dorm Admin");
    brandSub->setObjectName("BrandSub");
    brandColV->addWidget(brandName);
    brandColV->addWidget(brandSub);
    brandRow->addWidget(brandIcon);
    brandRow->addWidget(brandCol, 1);
    sideLay->addLayout(brandRow);
    sideLay->addSpacing(20);

    // Section label
    auto* menuLbl = new QLabel("NAVIGATION");
    menuLbl->setObjectName("SideMenuLabel");
    sideLay->addWidget(menuLbl);

    // Pages
    stack = new QStackedWidget;
    auto* dashboard = new DormAdminDashboardPage(uni, dormId);
    dashboard->setNavigator([this](int index){ switchPage(index); });
    stack->addWidget(dashboard);                                         // 0
    stack->addWidget(new DormAdminDormitoryPage(uni, dormId));            // 1
    stack->addWidget(new DormAdminRoomsPage(uni, dormId));                // 2
    stack->addWidget(new DormAdminStudentsPage(uni, dormId));             // 3
    stack->addWidget(new DormAdminRestaurantPage(uni, dormId));           // 4
    stack->addWidget(new DormAdminMealBookingPage(uni, dormId));          // 5
    stack->addWidget(new DormAdminActivityPage(uni, dormId, "Sports"));   // 6
    stack->addWidget(new DormAdminActivityPage(uni, dormId, "Cultural")); // 7
    stack->addWidget(new DormAdminHealthPage(uni, dormId));               // 8

    struct NavItem { const char* icon; const char* label; };
    NavItem items[] = {
        {"📊", "Dashboard"},
        {"🏢", "Dormitory"},
        {"🛏", "Rooms"},
        {"👥", "Students"},
        {"🍽", "Restaurant"},
        {"📅", "Meal Booking"},
        {"⚽", "Sports"},
        {"🎭", "Cultural"},
        {"🏥", "Health Clinic"},
    };
    for (int i = 0; i < 9; ++i)
        addNav(sideLay, items[i].icon, items[i].label, i);

    sideLay->addStretch();

    // Logout button
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: rgba(255,255,255,0.08);");
    sideLay->addWidget(sep);
    sideLay->addSpacing(8);

    auto* logoutBtn = new QPushButton("🚪  Sign Out");
    logoutBtn->setObjectName("NavBtn");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setFixedHeight(38);
    logoutBtn->setStyleSheet(
        "QPushButton { text-align: left; padding: 9px 14px; color: #F87171;"
        " background: transparent; border: none; border-radius: 8px;"
        " font-size: 13px; font-weight: 500; }"
        "QPushButton:hover { background: rgba(248,113,113,0.12); }");
    sideLay->addWidget(logoutBtn);
    sideLay->addSpacing(8);

    // Admin info at bottom
    auto* adminRow = new QHBoxLayout;
    auto* adminAv  = new QLabel("👤");
    adminAv->setFixedSize(34, 34);
    adminAv->setAlignment(Qt::AlignCenter);
    adminAv->setStyleSheet(
        "background:#2D3154; border-radius:10px; font-size:16px;");
    auto* adminCol  = new QWidget;
    auto* adminColV = new QVBoxLayout(adminCol);
    adminColV->setContentsMargins(0, 0, 0, 0); adminColV->setSpacing(1);
    auto* adminNameW = new QLabel(dormName);
    adminNameW->setObjectName("SideAdminName");
    auto* adminRoleW = new QLabel("Dorm Admin");
    adminRoleW->setObjectName("SideAdminRole");
    adminColV->addWidget(adminNameW);
    adminColV->addWidget(adminRoleW);
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

    // Connections
    connect(logoutBtn, &QPushButton::clicked, this, [this]{
        emit loggedOut();
    });
}

void DormAdminWindow::addNav(QVBoxLayout* sideLay, const QString& icon,
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

void DormAdminWindow::switchPage(int index) {
    for (int i = 0; i < navButtons.size(); ++i)
        navButtons[i]->setChecked(i == index);
    stack->setCurrentIndex(index);
    if (auto* r = dynamic_cast<DormAdminRefreshable*>(stack->widget(index)))
        r->refresh();
}

void DormAdminWindow::closeEvent(QCloseEvent* event) {
    uni.saveToFiles("data");
    event->accept();
}

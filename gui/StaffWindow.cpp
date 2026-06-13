#include "gui/StaffWindow.h"
#include "gui/StaffPages.h"
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

StaffWindow::StaffWindow(University& u, QWidget* parent)
    : QMainWindow(parent), uni(u)
{
    setWindowTitle("UDRMS – Dorms & Restaurant Staff");

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
    auto* brandIcon = new QLabel("🔧");
    brandIcon->setFixedSize(36, 36);
    brandIcon->setAlignment(Qt::AlignCenter);
    brandIcon->setStyleSheet(
        "background:#10B981; border-radius:10px; font-size:18px;");
    auto* brandCol = new QWidget;
    auto* brandColV = new QVBoxLayout(brandCol);
    brandColV->setContentsMargins(0, 0, 0, 0); brandColV->setSpacing(0);
    auto* brandName = new QLabel("UDRMS");
    brandName->setObjectName("Brand");
    auto* brandSub  = new QLabel("Staff Portal");
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
    stack->addWidget(new StaffDashboardPage(uni));      // 0
    stack->addWidget(new StaffMaintenancePage(uni));     // 1
    stack->addWidget(new StaffRestaurantPage(uni));      // 2

    struct NavItem { const char* icon; const char* label; };
    NavItem items[] = {
        {"📊", "Dashboard"},
        {"🔧", "Maintenance"},
        {"🍽", "Serve Meals"},
    };
    for (int i = 0; i < 3; ++i)
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

    // Staff info at bottom
    auto* staffRow = new QHBoxLayout;
    auto* staffAv  = new QLabel("👤");
    staffAv->setFixedSize(34, 34);
    staffAv->setAlignment(Qt::AlignCenter);
    staffAv->setStyleSheet(
        "background:#2D3154; border-radius:10px; font-size:16px;");
    auto* staffCol  = new QWidget;
    auto* staffColV = new QVBoxLayout(staffCol);
    staffColV->setContentsMargins(0, 0, 0, 0); staffColV->setSpacing(1);
    auto* staffNameW = new QLabel("Staff User");
    staffNameW->setObjectName("SideAdminName");
    auto* staffRoleW = new QLabel("Dorms & Restaurant");
    staffRoleW->setObjectName("SideAdminRole");
    staffColV->addWidget(staffNameW);
    staffColV->addWidget(staffRoleW);
    staffRow->addWidget(staffAv);
    staffRow->addWidget(staffCol, 1);
    sideLay->addLayout(staffRow);

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

    connect(logoutBtn, &QPushButton::clicked, this, [this]{
        emit loggedOut();
    });
}

void StaffWindow::addNav(QVBoxLayout* sideLay, const QString& icon,
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

void StaffWindow::switchPage(int index) {
    for (int i = 0; i < navButtons.size(); ++i)
        navButtons[i]->setChecked(i == index);
    stack->setCurrentIndex(index);
    if (auto* r = dynamic_cast<StaffRefreshable*>(stack->widget(index)))
        r->refresh();
}

void StaffWindow::closeEvent(QCloseEvent* event) {
    uni.saveToFiles("data");
    event->accept();
}

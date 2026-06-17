#include "gui/StudentWindow.h"
#include "gui/StudentPages.h"
#include "model/University.h"

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>

StudentWindow::StudentWindow(University& u, const QString& sid, QWidget* parent)
    : QMainWindow(parent), uni(u), studentId(sid)
{
    Student* s = currentStudent();
    QString name = s ? QString::fromStdString(s->getFullName()) : "Student";

    setWindowTitle("DORESM – Student Portal • " + name);

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
    auto* brandIcon = new QLabel("🎓");
    brandIcon->setFixedSize(36, 36);
    brandIcon->setAlignment(Qt::AlignCenter);
    brandIcon->setStyleSheet(
        "background:#4C6FFF; border-radius:10px; font-size:18px;");
    auto* brandCol = new QWidget;
    auto* brandColV = new QVBoxLayout(brandCol);
    brandColV->setContentsMargins(0, 0, 0, 0); brandColV->setSpacing(0);
    auto* brandName = new QLabel("DORESM");
    brandName->setObjectName("Brand");
    auto* brandSub  = new QLabel("Student Portal");
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
    stack->addWidget(new StudentDashboardPage(uni, studentId));     // 0
    stack->addWidget(new StudentEventsPage(uni, studentId));        // 1
    stack->addWidget(new StudentMealBookingPage(uni, studentId));   // 2
    stack->addWidget(new StudentHealthPage(uni, studentId));        // 3

    struct NavItem { const char* icon; const char* label; };
    NavItem items[] = {
        {"📊", "My Dashboard"},
        {"🎯", "Events"},
        {"🍽", "Meal Booking"},
        {"🏥", "Health Clinic"},
    };
    for (int i = 0; i < 4; ++i)
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

    // Student info at bottom
    auto* studentRow = new QHBoxLayout;
    auto* studentAv  = new QLabel("👤");
    studentAv->setFixedSize(34, 34);
    studentAv->setAlignment(Qt::AlignCenter);
    studentAv->setStyleSheet(
        "background:#2D3154; border-radius:10px; font-size:16px;");
    auto* studentCol  = new QWidget;
    auto* studentColV = new QVBoxLayout(studentCol);
    studentColV->setContentsMargins(0, 0, 0, 0); studentColV->setSpacing(1);
    auto* studentNameW = new QLabel(name);
    studentNameW->setObjectName("SideAdminName");
    auto* studentRoleW = new QLabel("Student");
    studentRoleW->setObjectName("SideAdminRole");
    studentColV->addWidget(studentNameW);
    studentColV->addWidget(studentRoleW);
    studentRow->addWidget(studentAv);
    studentRow->addWidget(studentCol, 1);
    sideLay->addLayout(studentRow);

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

void StudentWindow::addNav(QVBoxLayout* sideLay, const QString& icon,
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

void StudentWindow::switchPage(int index) {
    for (int i = 0; i < navButtons.size(); ++i)
        navButtons[i]->setChecked(i == index);
    stack->setCurrentIndex(index);
    if (auto* r = dynamic_cast<StudentRefreshable*>(stack->widget(index)))
        r->refresh();
}

Student* StudentWindow::currentStudent() {
    return uni.findStudent(studentId.toStdString());
}

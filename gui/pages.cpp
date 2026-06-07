#include "gui/pages.h"
#include "gui/widgets.h"
#include "model/University.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QStringList>
#include <QFrame>
#include <QScrollArea>


// ─── Helpers ─────────────────────────────────────────────────────────────────

namespace {

void clearLayout(QLayout* layout) {
    if (!layout) return;
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        if (item->layout()) clearLayout(item->layout());
        delete item;
    }
}

QTableWidget* makeTable(const QStringList& cols) {
    auto* t = new QTableWidget;
    t->setColumnCount(cols.size());
    t->setHorizontalHeaderLabels(cols);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->verticalHeader()->setVisible(false);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setShowGrid(false);
    t->setAlternatingRowColors(false);
    t->setFrameShape(QFrame::NoFrame);
    return t;
}

QString pickStudentId(QWidget* p, University& uni) {
    QStringList items;
    for (const auto& s : uni.getStudents())
        items << QString::fromStdString(s.getId() + " – " + s.getFullName());
    if (items.isEmpty()) { QMessageBox::warning(p, "Unavailable", "No students yet."); return {}; }
    bool ok = false;
    QString sel = QInputDialog::getItem(p, "Select Student", "Student:", items, 0, false, &ok);
    if (!ok) return {};
    return sel.section(" – ", 0, 0);
}

QFrame* hDivider() {
    auto* f = new QFrame;
    f->setFrameShape(QFrame::HLine);
    f->setStyleSheet("color:#F3F4F8;");
    return f;
}

// Colored progress bar
QProgressBar* makeOccupBar(int pct, const QString& colorHex) {
    auto* bar = new QProgressBar;
    bar->setValue(pct);
    bar->setTextVisible(false);
    bar->setFixedHeight(7);
    bar->setStyleSheet(
        "QProgressBar { background:#F1F2F4; border-radius:4px; border:none; }"
        "QProgressBar::chunk { background:" + colorHex + "; border-radius:4px; }");
    return bar;
}

} // namespace

// ─── Dashboard ───────────────────────────────────────────────────────────────

DashboardPage::DashboardPage(University& u, QWidget* parent)
    : QWidget(parent), uni(u)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void DashboardPage::refresh() {
    clearLayout(root);

    // Header
    root->addWidget(pageHeader("Good morning, Admin",
                               "Here's what's happening at ENSIA today."));

    // ── Stat row ──────────────────────────────────────────────────────
    auto* statRow = new QHBoxLayout;
    statRow->setSpacing(16);

    int meals = 0;
    for (const auto& d : uni.getDormitories())
        meals += d.getRestaurant().getMealsServed();

    statRow->addWidget(statCard("👥",
        QString::number(uni.totalStudents()), "Total Students",
        "+14 this week", "#EEF1FF"));
    statRow->addWidget(statCard("🏢",
        QString::number((int)uni.getDormitories().size()), "Dormitories",
        "3 fully occupied", "#DCFCE7", "#16A34A"));
    statRow->addWidget(statCard("🛏",
        QString::number(uni.availableRooms()), "Available Rooms",
        QString("Out of %1 total").arg(uni.totalRooms()), "#FEF3C7", "#D97706"));
    statRow->addWidget(statCard("🍽",
        QString::number(meals), "Meals Served Today",
        "+120 vs yesterday", "#FCE7F3", "#DB2777"));
    root->addLayout(statRow);

    // ── Modules section ───────────────────────────────────────────────
    auto* modLabel = new QLabel("Modules");
    modLabel->setObjectName("SectionTitle");
    root->addWidget(modLabel);

    auto* modGrid = new QGridLayout;
    modGrid->setSpacing(16);

    struct Mod { const char* emoji; const char* title; const char* desc; const char* bg; };
    Mod mods[] = {
        {"🏢", "Dormitory Management",
         "Manage multiple dormitories, capabilities, and room assignments.", "#EEF1FF"},
        {"🛏", "Room Management",
         "Track rooms, occupancy, and prevent over-allocation.", "#E0F2FE"},
        {"🍽", "Restaurant & Menus",
         "Daily breakfast, lunch, and dinner menu management.", "#FCE7F3"},
        {"📅", "Meal Booking",
         "Allow residents to book meals in advance.", "#DCFCE7"},
        {"⚽", "Sports Activities",
         "Register and manage sports programs for students.", "#F0FDF4"},
        {"🏥", "Health Clinic",
         "Manage appointments and health records for residents.", "#FFF7ED"},
    };
    for (int i = 0; i < 6; ++i) {
        auto* mc = moduleCard(mods[i].emoji, mods[i].title,
                              mods[i].desc, mods[i].bg);
        mc->setMinimumHeight(150);
        modGrid->addWidget(mc, i / 3, i % 3);
    }
    root->addLayout(modGrid);

    // ── Bottom row: Recent Activity + Right Panel ─────────────────────
    auto* bottomRow = new QHBoxLayout;
    bottomRow->setSpacing(16);

    // Recent Activity card
    auto* actCard = makeCard();
    actCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* actV = new QVBoxLayout(actCard);
    actV->setContentsMargins(0, 16, 0, 8);
    actV->setSpacing(0);

    auto* actHeader = new QHBoxLayout;
    actHeader->setContentsMargins(16, 0, 16, 12);
    auto* actTitle = new QLabel("Recent Activity");
    actTitle->setObjectName("SectionTitle");
    auto* viewAll = new QPushButton("View all");
    viewAll->setStyleSheet(
        "QPushButton { background:transparent; border:none; color:#4C6FFF;"
        "font-size:12px; font-weight:500; padding:0; }");
    actHeader->addWidget(actTitle);
    actHeader->addStretch();
    actHeader->addWidget(viewAll);
    actV->addLayout(actHeader);

    struct ARow { const char* emoji; const char* text; const char* time; };
    ARow rows[] = {
        {"👥", "Student Ahmed Benali assigned to Room 204 – Dormitory B", "2 min ago"},
        {"🍽", "Lunch menu updated for Dormitory A restaurant", "15 min ago"},
        {"📅", "Meal booking confirmed for 32 students – Dinner", "1 hr ago"},
        {"🏥", "Health clinic appointment scheduled – Layla Hamidi", "2 hr ago"},
        {"⚽", "Football training session registered – 18 students", "Yesterday"},
    };
    for (const auto& r : rows)
        actV->addWidget(actRow(r.emoji, r.text, r.time));

    bottomRow->addWidget(actCard, 3);

    // Right panel
    auto* rightCol = new QVBoxLayout;
    rightCol->setSpacing(16);

    // Dormitory Occupancy
    auto* occCard = makePanelCard();
    auto* occV = new QVBoxLayout(occCard);
    occV->setContentsMargins(18, 18, 18, 18);
    occV->setSpacing(10);
    auto* occTitle = new QLabel("Dormitory Occupancy");
    occTitle->setObjectName("PanelTitle");
    occV->addWidget(occTitle);

    const char* barColors[] = { "#4C6FFF", "#10B981", "#F59E0B", "#9CA3AF" };
    int ci = 0;
    for (const auto& d : uni.getDormitories()) {
        int cap = d.totalCapacity(), occ = d.totalOccupancy();
        int pct = cap ? occ * 100 / cap : 0;
        auto* nameRow = new QHBoxLayout;
        auto* dname = new QLabel(QString::fromStdString(d.getName()));
        dname->setStyleSheet("font-size:12px; font-weight:500; color:#374151;");
        auto* beds = new QLabel(QString("%1/%2 beds").arg(occ).arg(cap));
        beds->setStyleSheet("font-size:11px; color:#9CA3AF;");
        nameRow->addWidget(dname);
        nameRow->addStretch();
        nameRow->addWidget(beds);
        occV->addLayout(nameRow);
        occV->addWidget(makeOccupBar(pct, barColors[ci % 4]));
        auto* pctLbl = new QLabel(QString("%1% occupied").arg(pct));
        pctLbl->setStyleSheet("font-size:11px; color:#6B7280;");
        occV->addWidget(pctLbl);
        ++ci;
    }
    rightCol->addWidget(occCard);

    // Today's Menu card
    auto* menuPanelCard = new QFrame;
    menuPanelCard->setObjectName("MenuCard");
    auto* menuV = new QVBoxLayout(menuPanelCard);
    menuV->setContentsMargins(16, 14, 16, 14);
    menuV->setSpacing(6);
    auto* menuHdr = new QHBoxLayout;
    auto* menuTitle = new QLabel("🍽  Today's Menu");
    menuTitle->setObjectName("MenuTitle");
    auto* mngMenu = new QPushButton("Manage Menu →");
    mngMenu->setStyleSheet(
        "QPushButton { background:transparent; border:none; color:#D97706;"
        "font-size:11px; font-weight:500; padding:0; }");
    menuHdr->addWidget(menuTitle);
    menuHdr->addStretch();
    menuHdr->addWidget(mngMenu);
    menuV->addLayout(menuHdr);

    // Pull first dorm's menu as sample
    if (!uni.getDormitories().empty()) {
        const Menu& m = uni.getDormitories().front().getRestaurant().getMenu();
        menuV->addWidget(menuRow("🌅", "Breakfast", QString::fromStdString(m.breakfast)));
        menuV->addWidget(menuRow("☀️",  "Lunch",     QString::fromStdString(m.lunch)));
        menuV->addWidget(menuRow("🌙", "Dinner",    QString::fromStdString(m.dinner)));
    }
    rightCol->addWidget(menuPanelCard);

    // Upcoming Events
    auto* evCard = makePanelCard();
    auto* evV = new QVBoxLayout(evCard);
    evV->setContentsMargins(18, 18, 18, 18);
    evV->setSpacing(4);
    auto* evHdr = new QHBoxLayout;
    auto* evTitle = new QLabel("Upcoming Events"); evTitle->setObjectName("PanelTitle");
    auto* evAll   = new QPushButton("View all");
    evAll->setStyleSheet(
        "QPushButton { background:transparent; border:none; color:#4C6FFF;"
        "font-size:12px; padding:0; }");
    evHdr->addWidget(evTitle); evHdr->addStretch(); evHdr->addWidget(evAll);
    evV->addLayout(evHdr);
    evV->addWidget(eventRow("Today",          "Medical check-up day",   "🏥"));
    evV->addWidget(eventRow("Tomorrow",       "Basketball tournament",   "🏀"));
    evV->addWidget(eventRow("Sat, Jun 7",     "Cultural film evening",   "🎭"));
    evV->addWidget(eventRow("Mon, Jun 9",     "Room inspection – Block C","🛏"));
    rightCol->addWidget(evCard);
    rightCol->addStretch();

    bottomRow->addLayout(rightCol, 2);
    root->addLayout(bottomRow);
    root->addStretch();
}

// ─── Dormitories ─────────────────────────────────────────────────────────────

DormitoriesPage::DormitoriesPage(University& u, QWidget* parent)
    : QWidget(parent), uni(u)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void DormitoriesPage::refresh() {
    clearLayout(root);
    root->addWidget(pageHeader("Dormitories",
        "Manage dormitories, capacities, and attached restaurants."));

    // Stat row
    int totalBeds = 0, occupiedBeds = 0;
    for (const auto& d : uni.getDormitories()) {
        totalBeds    += d.totalCapacity();
        occupiedBeds += d.totalOccupancy();
    }
    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("🏢",
        QString::number((int)uni.getDormitories().size()),
        "Total Dormitories", "On campus", "#EEF1FF"));
    sRow->addWidget(statCard("🛏",
        QString::number(totalBeds), "Total Beds",
        "Across all dorms", "#DCFCE7"));
    sRow->addWidget(statCard("✅",
        QString::number(totalBeds - occupiedBeds),
        "Available Beds", "Ready to assign", "#FEF3C7"));
    sRow->addWidget(statCard("👥",
        QString::number(occupiedBeds),
        "Occupied Beds", "Current residents", "#FCE7F3"));
    root->addLayout(sRow);

    auto* gridLbl = new QLabel("All Dormitories");
    gridLbl->setObjectName("SectionTitle");
    root->addWidget(gridLbl);

    auto* grid = new QGridLayout; grid->setSpacing(16);
    int col = 0, row = 0;
    for (const auto& d : uni.getDormitories()) {
        int cap = d.totalCapacity(), occ = d.totalOccupancy();
        int pct = cap ? occ * 100 / cap : 0;

        auto* card = makeCard();
        auto* v = new QVBoxLayout(card);
        v->setContentsMargins(20, 20, 20, 20); v->setSpacing(10);

        // Header row
        auto* hr = new QHBoxLayout;
        auto* ic = new QLabel("🏢");
        ic->setFixedSize(44, 44); ic->setAlignment(Qt::AlignCenter);
        ic->setStyleSheet("background:#EEF1FF; border-radius:12px; font-size:20px;");
        auto* col2 = new QWidget;
        auto* col2V = new QVBoxLayout(col2);
        col2V->setContentsMargins(0, 0, 0, 0); col2V->setSpacing(2);
        auto* dName = new QLabel(QString::fromStdString(d.getName()));
        dName->setStyleSheet("font-size:15px; font-weight:700;");
        auto* dSub = new QLabel(QString("%1 rooms  ·  %2 beds")
            .arg(d.getRooms().size()).arg(cap));
        dSub->setStyleSheet("font-size:12px; color:#6B7280;");
        col2V->addWidget(dName);
        col2V->addWidget(dSub);
        hr->addWidget(ic); hr->addWidget(col2, 1);
        auto* badge = pill(pct >= 90 ? "Nearly Full" : pct == 0 ? "Empty" : "Active",
                           pct >= 90 ? "Full" : "Available");
        hr->addWidget(badge);
        v->addLayout(hr);

        v->addWidget(hDivider());

        // Stats row inside card
        auto* infoRow = new QHBoxLayout; infoRow->setSpacing(20);
        auto* mkInfo  = [](const QString& val, const QString& lbl) {
            auto* w = new QWidget;
            auto* vl = new QVBoxLayout(w);
            vl->setContentsMargins(0, 0, 0, 0); vl->setSpacing(2);
            auto* v2 = new QLabel(val); v2->setStyleSheet("font-size:18px; font-weight:700;");
            auto* l2 = new QLabel(lbl); l2->setStyleSheet("font-size:11px; color:#6B7280;");
            vl->addWidget(v2); vl->addWidget(l2);
            return w;
        };
        infoRow->addWidget(mkInfo(QString::number(occ), "Occupied"));
        infoRow->addWidget(mkInfo(QString::number(cap - occ), "Available"));
        infoRow->addWidget(mkInfo(QString::number(d.availableRoomCount()), "Free Rooms"));
        infoRow->addStretch();
        v->addLayout(infoRow);

        // Occupancy bar
        auto* pctRow = new QHBoxLayout;
        auto* pctLbl = new QLabel(QString("Occupancy: %1%").arg(pct));
        pctLbl->setStyleSheet("font-size:12px; color:#6B7280;");
        pctRow->addWidget(pctLbl);
        v->addLayout(pctRow);
        v->addWidget(makeOccupBar(pct, pct >= 90 ? "#EF4444" : pct >= 50 ? "#4C6FFF" : "#10B981"));

        // Restaurant
        auto* rRow = new QHBoxLayout;
        auto* rIc = new QLabel("🍽"); rIc->setStyleSheet("font-size:14px;");
        auto* rLbl = new QLabel(QString::fromStdString(d.getRestaurant().getName()));
        rLbl->setStyleSheet("font-size:12px; color:#374151;");
        auto* rServed = new QLabel(
            QString("Meals served today: %1").arg(d.getRestaurant().getMealsServed()));
        rServed->setStyleSheet("font-size:11px; color:#9CA3AF;");
        rRow->addWidget(rIc); rRow->addWidget(rLbl, 1); rRow->addWidget(rServed);
        v->addWidget(hDivider());
        v->addLayout(rRow);

        grid->addWidget(card, row, col);
        if (++col == 2) { col = 0; ++row; }
    }
    root->addLayout(grid);
    root->addStretch();
}

// ─── Rooms ────────────────────────────────────────────────────────────────────

RoomsPage::RoomsPage(University& u, QWidget* parent)
    : QWidget(parent), uni(u)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void RoomsPage::refresh() {
    clearLayout(root);

    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Room Management",
        "View, assign, and manage all dormitory rooms."));
    head->addStretch();
    auto* addBtn = new QPushButton("+ Assign Student");
    addBtn->setObjectName("Primary");
    head->addWidget(addBtn);
    root->addLayout(head);

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("🛏",  QString::number(uni.totalRooms()),
                             "Total Rooms",    "",  "#EEF1FF"));
    sRow->addWidget(statCard("✅",  QString::number(uni.availableRooms()),
                             "Available",      "",  "#DCFCE7"));
    sRow->addWidget(statCard("🔴", QString::number(uni.occupiedRooms()),
                             "Occupied",       "",  "#FEE2E2", "#EF4444"));
    root->addLayout(sRow);

    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);
    auto* table = makeTable({"Room", "Dormitory", "Type",
                              "Occupancy", "Status", "Residents"});
    int total = 0;
    for (const auto& d : uni.getDormitories()) total += (int)d.getRooms().size();
    table->setRowCount(total);
    table->setMinimumHeight(300);
    int i = 0;
    for (const auto& d : uni.getDormitories()) {
        for (const auto& rm : d.getRooms()) {
            table->setItem(i, 0, new QTableWidgetItem(
                QString::fromStdString(rm.getNumber())));
            table->setItem(i, 1, new QTableWidgetItem(
                QString::fromStdString(d.getName())));
            table->setItem(i, 2, new QTableWidgetItem(
                QString::fromStdString(rm.getType())));
            table->setItem(i, 3, new QTableWidgetItem(
                QString("%1/%2").arg(rm.getOccupancy()).arg(rm.getCapacity())));
            table->setCellWidget(i, 4, pill(
                QString::fromStdString(rm.status()),
                QString::fromStdString(rm.status())));
            QStringList names;
            for (const auto& occ : rm.getOccupants())
                names << QString::fromStdString(uni.studentName(occ));
            table->setItem(i, 5, new QTableWidgetItem(
                names.isEmpty() ? "No residents" : names.join(", ")));
            ++i;
        }
    }
    tv->addWidget(table);
    root->addWidget(tableCard);
    connect(addBtn, &QPushButton::clicked, this, [this]{ assignDialog(); });
}

void RoomsPage::assignDialog() {
    QString sid = pickStudentId(this, uni);
    if (sid.isEmpty()) return;
    QStringList dorms;
    for (const auto& d : uni.getDormitories())
        dorms << QString::fromStdString(d.getId());
    bool ok = false;
    QString dorm = QInputDialog::getItem(this, "Assign Student", "Dormitory:", dorms, 0, false, &ok);
    if (!ok) return;
    QStringList rooms;
    if (auto* d = uni.findDormitory(dorm.toStdString()))
        for (const auto& r : d->getRooms())
            if (!r.isFull() && !r.isUnderMaintenance())
                rooms << QString::fromStdString(r.getNumber());
    if (rooms.isEmpty()) {
        QMessageBox::warning(this, "Unavailable", "No available rooms.");
        return;
    }
    QString room = QInputDialog::getItem(this, "Assign Student", "Room:", rooms, 0, false, &ok);
    if (!ok) return;
    try {
        uni.assignStudentToRoom(sid.toStdString(), dorm.toStdString(), room.toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

// ─── Students ────────────────────────────────────────────────────────────────

StudentsPage::StudentsPage(University& u, QWidget* parent)
    : QWidget(parent), uni(u)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void StudentsPage::refresh() {
    clearLayout(root);

    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Students",
        "Manage student records and accommodation."));
    head->addStretch();
    auto* rmBtn  = new QPushButton("Remove Selected");
    auto* addBtn = new QPushButton("+ Add Student");
    addBtn->setObjectName("Primary");
    head->addWidget(rmBtn); head->addWidget(addBtn);
    root->addLayout(head);

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("👥", QString::number(uni.totalStudents()),
                             "Total Students", "Enrolled", "#EEF1FF"));
    sRow->addWidget(statCard("🛏",
        QString::number(uni.occupiedRooms()),
        "With Accommodation", "Assigned to room", "#DCFCE7"));
    sRow->addWidget(statCard("📋",
        QString::number(uni.totalStudents() - uni.occupiedRooms()),
        "Unassigned", "No room yet", "#FEF3C7", "#D97706"));
    root->addLayout(sRow);

    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);
    auto* table = makeTable({"ID", "Full Name", "Year", "Accommodation"});
    auto& studs = uni.getStudents();
    table->setRowCount((int)studs.size());
    table->setMinimumHeight(300);
    for (int i = 0; i < (int)studs.size(); ++i) {
        const auto& s = studs[i];
        table->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(s.getId())));
        table->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(s.getFullName())));
        table->setItem(i, 2, new QTableWidgetItem(
            QString("Year %1").arg(s.getAcademicYear())));
        table->setCellWidget(i, 3, pill(
            QString::fromStdString(s.accommodationStatus()),
            s.accommodationStatus() == "Assigned" ? "Available" : "Partial"));
    }
    tv->addWidget(table);
    root->addWidget(tableCard);

    connect(addBtn, &QPushButton::clicked, this, [this]{ addDialog(); });
    connect(rmBtn, &QPushButton::clicked, this, [this, table]{
        int r = table->currentRow();
        if (r < 0) {
            QMessageBox::information(this, "Remove", "Select a student row first.");
            return;
        }
        QString id = table->item(r, 0)->text();
        try { uni.removeStudent(id.toStdString()); refresh(); }
        catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what());
        }
    });
}

void StudentsPage::addDialog() {
    bool ok = false;
    QString id = QInputDialog::getText(this, "Add Student", "Student ID:",
                                       QLineEdit::Normal, "", &ok);
    if (!ok || id.trimmed().isEmpty()) return;
    QString name = QInputDialog::getText(this, "Add Student", "Full name:",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    int year = QInputDialog::getInt(this, "Add Student", "Academic year (1-5):",
                                    1, 1, 5, 1, &ok);
    if (!ok) return;
    try {
        uni.addStudent(Student(id.toStdString(), name.toStdString(), year));
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

// ─── Restaurant ───────────────────────────────────────────────────────────────

RestaurantPage::RestaurantPage(University& u, QWidget* parent)
    : QWidget(parent), uni(u)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void RestaurantPage::refresh() {
    clearLayout(root);
    root->addWidget(pageHeader("Restaurant & Menus",
        "Manage daily menus for each dormitory restaurant."));

    // Total meals stat
    int totalMeals = 0;
    for (const auto& d : uni.getDormitories())
        totalMeals += d.getRestaurant().getMealsServed();

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("🍽", QString::number(totalMeals),
                             "Total Meals Today", "All restaurants", "#FCE7F3"));
    sRow->addWidget(statCard("🏢",
        QString::number((int)uni.getDormitories().size()),
        "Restaurants", "Active", "#EEF1FF"));
    root->addLayout(sRow);

    for (auto& d : uni.getDormitories()) {
        auto* card = makeCard();
        auto* v = new QVBoxLayout(card);
        v->setContentsMargins(20, 20, 20, 20); v->setSpacing(12);

        auto* hdr = new QHBoxLayout;
        auto* ic = new QLabel("🍽");
        ic->setFixedSize(44, 44); ic->setAlignment(Qt::AlignCenter);
        ic->setStyleSheet("background:#FCE7F3; border-radius:12px; font-size:20px;");
        auto* rName = new QLabel(QString::fromStdString(d.getRestaurant().getName()));
        rName->setStyleSheet("font-size:15px; font-weight:700;");
        auto* rServed = new QLabel(
            QString("Meals served today: %1").arg(d.getRestaurant().getMealsServed()));
        rServed->setStyleSheet("font-size:12px; color:#6B7280;");
        hdr->addWidget(ic);
        auto* rCol = new QWidget; auto* rColV = new QVBoxLayout(rCol);
        rColV->setContentsMargins(0,0,0,0); rColV->setSpacing(2);
        rColV->addWidget(rName); rColV->addWidget(rServed);
        hdr->addWidget(rCol, 1);
        v->addLayout(hdr);
        v->addWidget(hDivider());

        const Menu& m = d.getRestaurant().getMenu();
        auto* mkField = [](const QString& lbl, const QString& val) {
            auto* w = new QWidget; auto* vl = new QVBoxLayout(w);
            vl->setContentsMargins(0,0,0,4); vl->setSpacing(4);
            auto* l = new QLabel(lbl);
            l->setStyleSheet("font-size:11px; font-weight:600; color:#6B7280;"
                             "text-transform:uppercase; letter-spacing:0.5px;");
            auto* e = new QLineEdit(val);
            vl->addWidget(l); vl->addWidget(e);
            return qMakePair(w, e);
        };
        auto bf  = mkField("Breakfast", QString::fromStdString(m.breakfast));
        auto lun = mkField("Lunch",     QString::fromStdString(m.lunch));
        auto din = mkField("Dinner",    QString::fromStdString(m.dinner));
        v->addWidget(bf.first);
        v->addWidget(lun.first);
        v->addWidget(din.first);

        auto* btnRow = new QHBoxLayout;
        auto* save  = new QPushButton("💾  Save Menu"); save->setObjectName("Primary");
        auto* serve = new QPushButton("+ Serve Meal");
        btnRow->addWidget(save); btnRow->addWidget(serve); btnRow->addStretch();
        v->addLayout(btnRow);

        std::string dormId = d.getId();
        auto* bfE  = bf.second;
        auto* lunE = lun.second;
        auto* dinE = din.second;
        connect(save, &QPushButton::clicked, this,
            [this, dormId, bfE, lunE, dinE]{
                if (auto* dd = uni.findDormitory(dormId))
                    dd->getRestaurant().setMenu(
                        Menu(bfE->text().toStdString(),
                             lunE->text().toStdString(),
                             dinE->text().toStdString()));
                QMessageBox::information(this, "Saved", "Menu updated.");
            });
        connect(serve, &QPushButton::clicked, this,
            [this, dormId]{
                if (auto* dd = uni.findDormitory(dormId))
                    dd->getRestaurant().serveMeal();
                refresh();
            });
        root->addWidget(card);
    }
    root->addStretch();
}

// ─── Meal Booking ─────────────────────────────────────────────────────────────

MealBookingPage::MealBookingPage(University& u, QWidget* parent)
    : QWidget(parent), uni(u)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void MealBookingPage::refresh() {
    clearLayout(root);
    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Meal Booking",
        "Residents can book breakfast, lunch, or dinner in advance."));
    head->addStretch();
    auto* btn = new QPushButton("+ Book Meal"); btn->setObjectName("Primary");
    head->addWidget(btn);
    root->addLayout(head);

    int confirmed = 0, pending = 0;
    for (const auto& bk : uni.getBookings())
        bk.confirmed ? ++confirmed : ++pending;

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("📅", QString::number((int)uni.getBookings().size()),
                             "Total Bookings", "All time", "#EEF1FF"));
    sRow->addWidget(statCard("✅", QString::number(confirmed),
                             "Confirmed", "", "#DCFCE7"));
    sRow->addWidget(statCard("⏳", QString::number(pending),
                             "Pending", "", "#FEF3C7", "#D97706"));
    root->addLayout(sRow);

    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);
    auto* table = makeTable({"Student", "Date", "Meal", "Status"});
    auto& bk = uni.getBookings();
    table->setRowCount((int)bk.size());
    table->setMinimumHeight(300);
    for (int i = 0; i < (int)bk.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(uni.studentName(bk[i].studentId))));
        table->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(bk[i].date)));
        table->setItem(i, 2, new QTableWidgetItem(
            QString::fromStdString(mealTypeToString(bk[i].meal))));
        table->setCellWidget(i, 3, pill(
            bk[i].confirmed ? "Confirmed" : "Pending",
            bk[i].confirmed ? "Available" : "Partial"));
    }
    tv->addWidget(table);
    root->addWidget(tableCard);
    connect(btn, &QPushButton::clicked, this, [this]{ bookDialog(); });
}

void MealBookingPage::bookDialog() {
    QString sid = pickStudentId(this, uni);
    if (sid.isEmpty()) return;
    bool ok = false;
    QString date = QInputDialog::getText(this, "Book Meal", "Date (YYYY-MM-DD):",
                                         QLineEdit::Normal, "2026-06-08", &ok);
    if (!ok || date.trimmed().isEmpty()) return;
    QString meal = QInputDialog::getItem(this, "Book Meal", "Meal:",
                                         {"Breakfast", "Lunch", "Dinner"}, 1, false, &ok);
    if (!ok) return;
    try {
        uni.bookMeal(MealBooking(sid.toStdString(), date.toStdString(),
                                  mealTypeFromString(meal.toStdString())));
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

// ─── Health Clinic ────────────────────────────────────────────────────────────

HealthPage::HealthPage(University& u, QWidget* parent)
    : QWidget(parent), uni(u)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void HealthPage::refresh() {
    clearLayout(root);
    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Health Clinic",
        "Manage appointments and student health records."));
    head->addStretch();
    auto* btn = new QPushButton("+ Schedule Appointment");
    btn->setObjectName("Primary");
    head->addWidget(btn);
    root->addLayout(head);

    int total = (int)uni.getClinic().getAppointments().size();
    int scheduled = 0;
    for (const auto& a : uni.getClinic().getAppointments())
        if (a.status == "Scheduled") ++scheduled;

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("🏥", QString::number(total),
                             "Total Appointments", "All time", "#EEF1FF"));
    sRow->addWidget(statCard("📋", QString::number(scheduled),
                             "Scheduled", "Upcoming", "#DCFCE7"));
    sRow->addWidget(statCard("✅", QString::number(total - scheduled),
                             "Completed", "Past", "#FEF3C7", "#D97706"));
    root->addLayout(sRow);

    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);
    auto* table = makeTable({"Student", "Date", "Time", "Reason", "Status"});
    auto& ap = uni.getClinic().getAppointments();
    table->setRowCount((int)ap.size());
    table->setMinimumHeight(300);
    for (int i = 0; i < (int)ap.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(uni.studentName(ap[i].studentId))));
        table->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(ap[i].date)));
        table->setItem(i, 2, new QTableWidgetItem(
            QString::fromStdString(ap[i].time)));
        table->setItem(i, 3, new QTableWidgetItem(
            QString::fromStdString(ap[i].reason)));
        table->setCellWidget(i, 4, pill(
            QString::fromStdString(ap[i].status),
            ap[i].status == "Scheduled" ? "Partial" : "Available"));
    }
    tv->addWidget(table);
    root->addWidget(tableCard);
    connect(btn, &QPushButton::clicked, this, [this]{ scheduleDialog(); });
}

void HealthPage::scheduleDialog() {
    QString sid = pickStudentId(this, uni);
    if (sid.isEmpty()) return;
    bool ok = false;
    QString date = QInputDialog::getText(this, "Schedule", "Date (YYYY-MM-DD):",
                                         QLineEdit::Normal, "2026-06-08", &ok);
    if (!ok || date.trimmed().isEmpty()) return;
    QString time = QInputDialog::getText(this, "Schedule", "Time (HH:MM):",
                                         QLineEdit::Normal, "10:00", &ok);
    if (!ok) return;
    QString reason = QInputDialog::getText(this, "Schedule", "Reason:",
                                           QLineEdit::Normal, "Check-up", &ok);
    if (!ok) return;
    try {
        uni.getClinic().schedule(
            Appointment(sid.toStdString(), date.toStdString(),
                        time.toStdString(), reason.toStdString()));
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

// ─── Activities ───────────────────────────────────────────────────────────────

ActivityPage::ActivityPage(University& u, QString cat, QWidget* parent)
    : QWidget(parent), uni(u), category(std::move(cat))
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void ActivityPage::refresh() {
    clearLayout(root);
    const bool sports = (category == "Sports");

    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader(
        sports ? "Sports Activities" : "Cultural Activities",
        sports ? "Register and manage sports programs for students."
               : "Manage cultural and recreational activities."));
    head->addStretch();
    auto* enrollBtn = new QPushButton("Enroll Student");
    auto* addBtn    = new QPushButton("+ Add Activity");
    addBtn->setObjectName("Primary");
    head->addWidget(enrollBtn); head->addWidget(addBtn);
    root->addLayout(head);

    // Count
    int count = 0, enrolled = 0;
    for (const auto& a : uni.getActivities()) {
        if (QString::fromStdString(a->category()) != category) continue;
        ++count;
        enrolled += (int)a->getParticipants().size();
    }

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard(sports ? "⚽" : "🎭",
        QString::number(count), "Activities", "Registered",
        sports ? "#F0FDF4" : "#FFF7ED"));
    sRow->addWidget(statCard("👥", QString::number(enrolled),
                             "Participants", "Enrolled", "#EEF1FF"));
    root->addLayout(sRow);

    auto* grid = new QGridLayout; grid->setSpacing(16);
    int col = 0, row = 0;
    for (const auto& a : uni.getActivities()) {
        if (QString::fromStdString(a->category()) != category) continue;

        auto* card = makeCard();
        auto* v = new QVBoxLayout(card);
        v->setContentsMargins(20, 20, 20, 20); v->setSpacing(10);

        auto* hr = new QHBoxLayout;
        auto* ic = new QLabel(sports ? "⚽" : "🎭");
        ic->setFixedSize(44, 44); ic->setAlignment(Qt::AlignCenter);
        ic->setStyleSheet(QString("background:") +
                          (sports ? "#F0FDF4" : "#FFF7ED") +
                          "; border-radius:12px; font-size:20px;");
        auto* aName = new QLabel(QString::fromStdString(a->getName()));
        aName->setStyleSheet("font-size:15px; font-weight:700;");
        hr->addWidget(ic); hr->addWidget(aName, 1);
        hr->addWidget(pill(QString("%1 enrolled")
            .arg(a->getParticipants().size()), "Available"));
        v->addLayout(hr);

        auto* desc = new QLabel(QString::fromStdString(a->describe()));
        desc->setStyleSheet("font-size:12px; color:#6B7280;");
        desc->setWordWrap(true);
        v->addWidget(desc);

        if (!a->getParticipants().empty()) {
            v->addWidget(hDivider());
            auto* plbl = new QLabel("Participants:");
            plbl->setStyleSheet("font-size:11px; font-weight:600; color:#6B7280;");
            v->addWidget(plbl);
            QStringList names;
            for (const auto& pid : a->getParticipants())
                names << QString::fromStdString(uni.studentName(pid));
            auto* pnames = new QLabel(names.join(", "));
            pnames->setStyleSheet("font-size:12px; color:#374151;");
            pnames->setWordWrap(true);
            v->addWidget(pnames);
        }

        grid->addWidget(card, row, col);
        if (++col == 2) { col = 0; ++row; }
    }

    if (count == 0) {
        auto* empty = makeCard();
        auto* ev = new QVBoxLayout(empty);
        ev->setContentsMargins(40, 40, 40, 40);
        auto* et = new QLabel(sports
            ? "No sports activities yet.\nClick \"+ Add Activity\" to create one."
            : "No cultural activities yet.\nClick \"+ Add Activity\" to create one.");
        et->setAlignment(Qt::AlignCenter);
        et->setStyleSheet("color:#9CA3AF; font-size:14px;");
        ev->addWidget(et);
        root->addWidget(empty);
    } else {
        root->addLayout(grid);
    }
    root->addStretch();

    connect(addBtn,    &QPushButton::clicked, this, [this]{ addDialog(); });
    connect(enrollBtn, &QPushButton::clicked, this, [this]{ enrollDialog(); });
}

void ActivityPage::addDialog() {
    const bool sports = (category == "Sports");
    bool ok = false;
    QString name = QInputDialog::getText(this, "Add Activity", "Name:",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    QString sched = QInputDialog::getText(this, "Add Activity", "Schedule:",
                                          QLineEdit::Normal, "", &ok);
    if (!ok) return;
    QString extra = QInputDialog::getText(this, "Add Activity",
                                          sports ? "Coach:" : "Venue:",
                                          QLineEdit::Normal, "", &ok);
    if (!ok) return;
    if (sports)
        uni.addActivity(std::make_unique<SportsActivity>(
            name.toStdString(), sched.toStdString(), extra.toStdString()));
    else
        uni.addActivity(std::make_unique<CulturalActivity>(
            name.toStdString(), sched.toStdString(), extra.toStdString()));
    refresh();
}

void ActivityPage::enrollDialog() {
    QStringList acts;
    for (const auto& a : uni.getActivities())
        if (QString::fromStdString(a->category()) == category)
            acts << QString::fromStdString(a->getName());
    if (acts.isEmpty()) {
        QMessageBox::warning(this, "Unavailable", "No activities yet.");
        return;
    }
    bool ok = false;
    QString an = QInputDialog::getItem(this, "Enroll", "Activity:", acts, 0, false, &ok);
    if (!ok) return;
    QString sid = pickStudentId(this, uni);
    if (sid.isEmpty()) return;
    for (const auto& a : uni.getActivities()) {
        if (QString::fromStdString(a->getName()) == an &&
            QString::fromStdString(a->category()) == category)
        {
            try { a->enroll(sid.toStdString()); }
            catch (const std::exception& e) {
                QMessageBox::warning(this, "Error", e.what());
            }
            break;
        }
    }
    refresh();
}
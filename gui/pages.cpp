#include "gui/pages.h"
#include "gui/widgets.h"
#include "gui/Validation.h"
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
#include <QDate>
#include <QComboBox>
#include <QSpinBox>
#include <algorithm>


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
    t->verticalHeader()->setDefaultSectionSize(50);
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

QString relDateLabel(const QString& iso) {
    QDate d = QDate::fromString(iso, "yyyy-MM-dd");
    if (!d.isValid()) return iso;
    QDate today = QDate::currentDate();
    if (d == today)            return "Today";
    if (d == today.addDays(1)) return "Tomorrow";
    return d.toString("ddd, MMM d");
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

void DashboardPage::setNavigator(std::function<void(int)> nav) {
    navigate = std::move(nav);
}

void DashboardPage::refresh() {
    clearLayout(root);

    // Header
    root->addWidget(pageHeader("Good morning, Admin",
                               "Here's what's happening at University today."));

    // ── Stat row ──────────────────────────────────────────────────────
    auto* statRow = new QHBoxLayout;
    statRow->setSpacing(16);

    int meals = uni.getWeeklyMenu().days[std::max(0, QDate::currentDate().dayOfWeek() - 1)].mealsServed;

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
const int modTargets[6] = {1, 2, 4, 5, 6, 7};  // dorm, rooms, restaurant, booking, sports, health
for (int i = 0; i < 6; ++i) {
    int target = modTargets[i];
    auto* mc = moduleCard(mods[i].emoji, mods[i].title,
                          mods[i].desc, mods[i].bg,
                          [this, target]{ if (navigate) navigate(target); });
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
    connect(viewAll, &QPushButton::clicked, this, [this]{
    QMessageBox::information(this, "Recent Activity",
        "The full activity history will appear here.");
});

const auto& logEntries = uni.getActivityLog();
if (logEntries.empty()) {
    auto* none = new QLabel("  No recent activity yet — actions you take will show up here.");
    none->setStyleSheet("color:#9CA3AF; font-size:13px; padding:8px 16px 16px;");
    actV->addWidget(none);
} else {
    int shown = 0;
    for (const auto& e : logEntries) {
        if (shown++ >= 6) break;
        actV->addWidget(actRow(QString::fromStdString(e.emoji),
                               QString::fromStdString(e.text),
                               QString::fromStdString(e.time)));
    }
}

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
    connect(mngMenu, &QPushButton::clicked, this, [this]{ if (navigate) navigate(4); });

    int dayOfWeek = std::max(0, QDate::currentDate().dayOfWeek() - 1);
    const auto& dailyMenu = uni.getWeeklyMenu().days[dayOfWeek];
    menuV->addWidget(menuRow("🌅", "Breakfast", QString::fromStdString(dailyMenu.breakfast)));
    menuV->addWidget(menuRow("☀️",  "Lunch",     QString::fromStdString(dailyMenu.lunch)));
    menuV->addWidget(menuRow("🌙", "Dinner",    QString::fromStdString(dailyMenu.dinner)));
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
    connect(evAll, &QPushButton::clicked, this, [this]{
    QMessageBox::information(this, "Upcoming Events",
        "The full events calendar will appear here.");
});
    {
    struct Ev { QString date; QString name; QString emoji; };
    std::vector<Ev> evs;
    const QString today = QDate::currentDate().toString("yyyy-MM-dd");

    // Health clinic appointments
    for (const auto& a : uni.getClinic().getAppointments()) {
        if (a.status != "Scheduled") continue;
        QString date = QString::fromStdString(a.date);
        if (date < today) continue;
        evs.push_back({ date,
            QString("%1 – %2").arg(QString::fromStdString(a.reason),
                                   QString::fromStdString(uni.studentName(a.studentId))),
            "🏥" });
    }

    // Sports and Cultural activities
    for (const auto& a : uni.getActivities()) {
        QString sched = QString::fromStdString(a->getSchedule());
        QString cat = QString::fromStdString(a->category());
        QString actEmoji = (cat == "Sports") ? "⚽" : "🎭";
        int approved = 0;
        for (const auto& e : a->getEnrollments())
            if (e.status == "Approved") ++approved;
        evs.push_back({ today,
            QString("%1 – %2 (%3 enrolled)").arg(
                QString::fromStdString(a->getName()), sched,
                QString::number(approved)),
            actEmoji });
    }

    std::sort(evs.begin(), evs.end(),
              [](const Ev& a, const Ev& b){ return a.date < b.date; });

    if (evs.empty()) {
        auto* none = new QLabel("No upcoming events.");
        none->setStyleSheet("font-size:12px; color:#9CA3AF;");
        evV->addWidget(none);
    } else {
        int shown = 0;
        for (const auto& e : evs) {
            if (shown++ >= 5) break;
            evV->addWidget(eventRow(relDateLabel(e.date), e.name, e.emoji));
        }
    }
}
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
auto* head = new QHBoxLayout;
head->addWidget(pageHeader("Dormitories",
    "Manage dormitories, capacities, and attached restaurants."));
head->addStretch();
auto* addDormBtn = new QPushButton("+ Add Dormitory");
addDormBtn->setObjectName("Primary");
head->addWidget(addDormBtn);
root->addLayout(head);
connect(addDormBtn, &QPushButton::clicked, this, [this]{ addDormitoryDialog(); });

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
        std::function<QWidget*(const QString&, const QString&)> mkInfo  = [](const QString& val, const QString& lbl) -> QWidget* {
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
        auto* rServed = new QLabel("Serves central weekly menu");
        rServed->setStyleSheet("font-size:11px; color:#9CA3AF;");
        rRow->addWidget(rIc); rRow->addWidget(rLbl, 1); rRow->addWidget(rServed);
        v->addWidget(hDivider());
        v->addLayout(rRow);

        v->addWidget(hDivider());
auto* actionRow = new QHBoxLayout;
auto* addRoomBtn = new QPushButton("+ Room");
auto* delRoomBtn = new QPushButton("Delete Room");
auto* delDormBtn = new QPushButton("Delete Dorm");
actionRow->addWidget(addRoomBtn);
actionRow->addWidget(delRoomBtn);
actionRow->addStretch();
actionRow->addWidget(delDormBtn);
v->addLayout(actionRow);

QString dormId = QString::fromStdString(d.getId());
connect(addRoomBtn, &QPushButton::clicked, this, [this, dormId]{ addRoomDialog(dormId); });
connect(delRoomBtn, &QPushButton::clicked, this, [this, dormId]{ deleteRoomDialog(dormId); });
connect(delDormBtn, &QPushButton::clicked, this, [this, dormId]{ deleteDormitoryDialog(dormId); });
        grid->addWidget(card, row, col);
        if (++col == 2) { col = 0; ++row; }
    }
    root->addLayout(grid);
    root->addStretch();
}

void DormitoriesPage::addDormitoryDialog() {
    bool ok = false;
    QString id;
    while (true) {
        id = QInputDialog::getText(this, "Add Dormitory",
            "Dormitory ID (single uppercase letter, e.g. E):",
            QLineEdit::Normal, id, &ok);
        if (!ok) return;
        QString err = Validate::dormitoryId(id);
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid Dormitory ID", err);
    }
    QString name;
    name = "Dormitory " + id.trimmed();
    while (true) {
        name = QInputDialog::getText(this, "Add Dormitory", "Dormitory name:",
            QLineEdit::Normal, name, &ok);
        if (!ok) return;
        QString err = Validate::dormitoryName(name);
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid Dormitory Name", err);
    }
    try {
        uni.addDormitory(Dormitory(id.trimmed().toStdString(), name.trimmed().toStdString()));
        uni.logActivity("🏢", ("Added dormitory " + name.trimmed()).toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void DormitoriesPage::deleteDormitoryDialog(const QString& dormId) {
    auto* d = uni.findDormitory(dormId.toStdString());
    if (!d) return;
    QString nm = QString::fromStdString(d->getName());
    if (QMessageBox::question(this, "Delete Dormitory",
            QString("Delete \"%1\" and all its rooms?\nAny residents will be unassigned.").arg(nm))
        != QMessageBox::Yes) return;
    try {
        uni.removeDormitory(dormId.toStdString());
        uni.logActivity("🗑", ("Deleted dormitory " + nm).toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void DormitoriesPage::addRoomDialog(const QString& dormId) {
    auto* d = uni.findDormitory(dormId.toStdString());
    if (!d) return;
    bool ok = false;
    QString num;
    while (true) {
        num = QInputDialog::getText(this, "Add Room",
            QString("Room number (format: %1-NNN, e.g. %1-101):").arg(dormId),
            QLineEdit::Normal, num.isEmpty() ? dormId + "-" : num, &ok);
        if (!ok) return;
        QString err = Validate::roomNumber(num, dormId);
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid Room Number", err);
    }
    int cap = QInputDialog::getInt(this, "Add Room", "Capacity (1-3):", 1, 1, 3, 1, &ok);
    if (!ok) return;
    QString type = cap == 1 ? "Single" : cap == 2 ? "Double" : "Triple";
    try {
        if (d->findRoom(num.trimmed().toStdString()))
            throw std::runtime_error("Room already exists: " + num.trimmed().toStdString());
        d->addRoom(Room(num.trimmed().toStdString(), cap, type.toStdString()));
        uni.logActivity("🛏", ("Added room " + num.trimmed() + " to " +
                               QString::fromStdString(d->getName())).toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void DormitoriesPage::deleteRoomDialog(const QString& dormId) {
    auto* d = uni.findDormitory(dormId.toStdString());
    if (!d) return;
    QStringList rooms;
    for (const auto& r : d->getRooms()) rooms << QString::fromStdString(r.getNumber());
    if (rooms.isEmpty()) { QMessageBox::information(this, "Delete Room", "No rooms to delete."); return; }
    bool ok = false;
    QString num = QInputDialog::getItem(this, "Delete Room", "Room:", rooms, 0, false, &ok);
    if (!ok) return;
    try {
        d->removeRoom(num.toStdString());   // throws if occupied
        uni.logActivity("🗑", ("Deleted room " + num + " from " +
                               QString::fromStdString(d->getName())).toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
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

void RoomsPage::applyFilters(QTableWidget* table, QComboBox* dormBox,
                             QComboBox* typeBox, QComboBox* statusBox,
                             QLineEdit* studentEdit, QLineEdit* roomEdit) {
    QString sId   = studentEdit->text().trimmed().toLower();
    QString sRoom = roomEdit->text().trimmed().toLower();
    QString sDorm = dormBox->currentIndex() == 0 ? "" : dormBox->currentText();
    QString sType = typeBox->currentIndex() == 0 ? "" : typeBox->currentText();
    QString sStat = statusBox->currentIndex() == 0 ? "" : statusBox->currentText();

    // Persist filter values for across-refresh continuity
    fStudentId = studentEdit->text();
    fRoom      = roomEdit->text();
    fDormIdx   = dormBox->currentIndex();
    fTypeIdx   = typeBox->currentIndex();
    fStatusIdx = statusBox->currentIndex();

    int visible = 0;
    for (int r = 0; r < table->rowCount(); ++r) {
        bool show = true;

        // Student ID filter — search in column 5 (Residents) and check occupant IDs stored in column 6
        if (!sId.isEmpty() && table->item(r, 6)) {
            QString ids = table->item(r, 6)->text().toLower();
            if (!ids.contains(sId)) show = false;
        }
        // Room number filter — column 0
        if (show && !sRoom.isEmpty() && table->item(r, 0)) {
            if (!table->item(r, 0)->text().toLower().contains(sRoom)) show = false;
        }
        // Dormitory filter — column 1
        if (show && !sDorm.isEmpty() && table->item(r, 1)) {
            if (table->item(r, 1)->text() != sDorm) show = false;
        }
        // Type filter — column 2
        if (show && !sType.isEmpty() && table->item(r, 2)) {
            if (table->item(r, 2)->text() != sType) show = false;
        }
        // Status filter — column 4 (stored as hidden text in column 7)
        if (show && !sStat.isEmpty() && table->item(r, 7)) {
            if (table->item(r, 7)->text() != sStat) show = false;
        }

        table->setRowHidden(r, !show);
        if (show) ++visible;
    }

    // Update result count label — it's stored as the table card's property
    if (auto* countLbl = table->property("countLabel").value<QLabel*>()) {
        countLbl->setText(QString("Showing %1 of %2 rooms").arg(visible).arg(table->rowCount()));
    }
}

void RoomsPage::refresh() {
    clearLayout(root);

    // ── Header with action buttons ───────────────────────────────────
    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Room Management",
        "View, assign, reassign, and manage all dormitory rooms."));
    head->addStretch();

    auto* maintBtn    = new QPushButton("🔧 Toggle Maintenance");
    auto* unassignBtn = new QPushButton("Remove from Room");
    auto* reassignBtn = new QPushButton("🔄  Reassign");
    auto* assignBtn   = new QPushButton("+ Assign Student");
    assignBtn->setObjectName("Primary");

    reassignBtn->setStyleSheet(
        "QPushButton { background:#EEF1FF; color:#4C6FFF; border:1px solid #D1D5DB;"
        "border-radius:8px; padding:7px 16px; font-weight:500; }"
        "QPushButton:hover { background:#DDE3FF; }");

    maintBtn->setStyleSheet(
        "QPushButton { background:#FEF3C7; color:#D97706; border:1px solid #FDE68A;"
        "border-radius:8px; padding:7px 16px; font-weight:500; }"
        "QPushButton:hover { background:#FDE68A; }");

    head->addWidget(maintBtn);
    head->addWidget(unassignBtn);
    head->addWidget(reassignBtn);
    head->addWidget(assignBtn);
    root->addLayout(head);

    // ── Stat cards row ───────────────────────────────────────────────
    int maintRooms = 0;
    for (const auto& d : uni.getDormitories())
        for (const auto& r : d.getRooms())
            if (r.isUnderMaintenance()) ++maintRooms;

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("🛏",  QString::number(uni.totalRooms()),
                             "Total Rooms",    "All dormitories",  "#EEF1FF"));
    sRow->addWidget(statCard("✅",  QString::number(uni.availableRooms()),
                             "Available",      "Ready to assign",  "#DCFCE7"));
    sRow->addWidget(statCard("🔴", QString::number(uni.occupiedRooms()),
                             "Occupied",       "Currently in use", "#FEE2E2", "#EF4444"));
    sRow->addWidget(statCard("🔧", QString::number(maintRooms),
                             "Maintenance",    "Under repair",     "#FEF3C7", "#D97706"));
    root->addLayout(sRow);

    // ── Filter bar ───────────────────────────────────────────────────
    auto* filterCard = makeCard();
    auto* filterLay = new QVBoxLayout(filterCard);
    filterLay->setContentsMargins(20, 16, 20, 16);
    filterLay->setSpacing(12);

    // Filter title row
    auto* filterHeader = new QHBoxLayout;
    auto* filterIcon = new QLabel("🔍");
    filterIcon->setStyleSheet("font-size:14px;");
    auto* filterTitle = new QLabel("Filters");
    filterTitle->setStyleSheet("font-size:14px; font-weight:600; color:#374151;");
    filterHeader->addWidget(filterIcon);
    filterHeader->addWidget(filterTitle);
    filterHeader->addStretch();
    auto* clearBtn = new QPushButton("Clear Filters");
    clearBtn->setStyleSheet(
        "QPushButton { background:transparent; border:none; color:#EF4444;"
        "font-size:12px; font-weight:500; padding:0; }"
        "QPushButton:hover { color:#DC2626; }");
    clearBtn->setCursor(Qt::PointingHandCursor);
    filterHeader->addWidget(clearBtn);
    filterLay->addLayout(filterHeader);

    // Filter controls row
    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(12);

    // Student ID search
    auto* studentIdEdit = new QLineEdit;
    studentIdEdit->setPlaceholderText("Search by Student ID...");
    studentIdEdit->setMinimumWidth(140);
    studentIdEdit->setText(fStudentId);

    // Dormitory combo
    auto* dormCombo = new QComboBox;
    dormCombo->addItem("All Dormitories");
    for (const auto& d : uni.getDormitories())
        dormCombo->addItem(QString::fromStdString(d.getName()));
    dormCombo->setMinimumWidth(140);
    if (fDormIdx < dormCombo->count()) dormCombo->setCurrentIndex(fDormIdx);

    // Room number search
    auto* roomEdit = new QLineEdit;
    roomEdit->setPlaceholderText("Room number...");
    roomEdit->setMinimumWidth(120);
    roomEdit->setText(fRoom);

    // Type combo
    auto* typeCombo = new QComboBox;
    typeCombo->addItems({"All Types", "Single", "Double", "Triple"});
    typeCombo->setMinimumWidth(110);
    if (fTypeIdx < typeCombo->count()) typeCombo->setCurrentIndex(fTypeIdx);

    // Status combo
    auto* statusCombo = new QComboBox;
    statusCombo->addItems({"All Status", "Available", "Partial", "Full", "Maintenance"});
    statusCombo->setMinimumWidth(120);
    if (fStatusIdx < statusCombo->count()) statusCombo->setCurrentIndex(fStatusIdx);

    filterRow->addWidget(studentIdEdit);
    filterRow->addWidget(dormCombo);
    filterRow->addWidget(roomEdit);
    filterRow->addWidget(typeCombo);
    filterRow->addWidget(statusCombo);
    filterLay->addLayout(filterRow);

    root->addWidget(filterCard);

    // ── Result count label ───────────────────────────────────────────
    auto* countLbl = new QLabel;
    countLbl->setStyleSheet("font-size:12px; color:#6B7280; padding:0 2px;");
    root->addWidget(countLbl);

    // ── Table inside card ────────────────────────────────────────────
    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);

    // Visible columns: Room, Dormitory, Type, Occupancy, Status, Residents
    // Hidden columns: 6 = student IDs (for filtering), 7 = raw status text
    auto* table = makeTable({"Room", "Dormitory", "Type",
                              "Occupancy", "Status", "Residents",
                              "StudentIDs", "RawStatus"});
    table->setColumnHidden(6, true);   // hidden: student IDs for filter
    table->setColumnHidden(7, true);   // hidden: raw status text for filter

    // Store countLbl as a property so applyFilters can update it
    table->setProperty("countLabel", QVariant::fromValue(countLbl));

    int totalRows = 0;
    for (const auto& d : uni.getDormitories()) totalRows += (int)d.getRooms().size();
    table->setRowCount(totalRows);
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

            QString statusText = QString::fromStdString(rm.status());
            table->setCellWidget(i, 4, pill(statusText, statusText));

            QStringList names, ids;
            for (const auto& occ : rm.getOccupants()) {
                names << QString::fromStdString(uni.studentName(occ));
                ids   << QString::fromStdString(occ);
            }
            table->setItem(i, 5, new QTableWidgetItem(
                names.isEmpty() ? "—" : names.join(", ")));

            // Hidden columns for filtering
            table->setItem(i, 6, new QTableWidgetItem(ids.join(" ")));
            table->setItem(i, 7, new QTableWidgetItem(statusText));
            ++i;
        }
    }

    tv->addWidget(table);
    root->addWidget(tableCard);

    // ── Connect filter signals ───────────────────────────────────────
    auto doFilter = [this, table, dormCombo, typeCombo, statusCombo, studentIdEdit, roomEdit]{
        applyFilters(table, dormCombo, typeCombo, statusCombo, studentIdEdit, roomEdit);
    };
    connect(studentIdEdit, &QLineEdit::textChanged, this, doFilter);
    connect(roomEdit,      &QLineEdit::textChanged, this, doFilter);
    connect(dormCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, doFilter);
    connect(typeCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, doFilter);
    connect(statusCombo,QOverload<int>::of(&QComboBox::currentIndexChanged), this, doFilter);

    connect(clearBtn, &QPushButton::clicked, this, [=]{
        studentIdEdit->clear();
        roomEdit->clear();
        dormCombo->setCurrentIndex(0);
        typeCombo->setCurrentIndex(0);
        statusCombo->setCurrentIndex(0);
    });

    // Apply any persisted filters
    doFilter();

    // ── Connect action buttons ───────────────────────────────────────
    connect(assignBtn,   &QPushButton::clicked, this, [this]{ assignDialog(); });
    connect(reassignBtn, &QPushButton::clicked, this, [this]{ reassignDialog(); });
    connect(unassignBtn, &QPushButton::clicked, this, [this]{ unassignDialog(); });
    connect(maintBtn,    &QPushButton::clicked, this, [this, table]{ toggleMaintenanceDialog(table); });
}

void RoomsPage::toggleMaintenanceDialog(QTableWidget* table) {
    if (!table) return;
    int r = table->currentRow();
    if (r < 0) {
        QMessageBox::information(this, "Select Room", "Please select a room first.");
        return;
    }
    QString roomNo = table->item(r, 0)->text();
    QString dormName = table->item(r, 1)->text();

    Dormitory* targetDorm = nullptr;
    for (auto& d : uni.getDormitories()) {
        if (QString::fromStdString(d.getName()) == dormName) {
            targetDorm = &d;
            break;
        }
    }
    if (!targetDorm) return;

    Room* room = targetDorm->findRoom(roomNo.toStdString());
    if (!room) return;

    if (!room->isUnderMaintenance()) {
        if (!room->isEmpty()) {
            QMessageBox::warning(this, "Cannot enter maintenance",
                "You have to reassign them to another room first before putting this room in maintenance mode.");
            return;
        }
        room->setMaintenance(true);
        uni.logActivity("🔧", "Placed room " + roomNo.toStdString() + " in maintenance");
    } else {
        room->setMaintenance(false);
        uni.logActivity("✅", "Removed room " + roomNo.toStdString() + " from maintenance");
    }
    refresh();
}

void RoomsPage::assignDialog() {
    QString sid = pickStudentId(this, uni);
    if (sid.isEmpty()) return;

    // Check if student is already accommodated
    if (auto* s = uni.findStudent(sid.toStdString())) {
        if (s->isAccommodated()) {
            QMessageBox::warning(this, "Already Assigned",
                QString("Student %1 is already assigned to Room %2 in Dormitory %3.\n"
                        "Use Reassign to move them.")
                    .arg(sid,
                         QString::fromStdString(s->getRoomNumber()),
                         QString::fromStdString(s->getDormitoryId())));
            return;
        }
    }

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
        QMessageBox::warning(this, "Unavailable", "No available rooms in this dormitory.");
        return;
    }
    QString room = QInputDialog::getItem(this, "Assign Student", "Room:", rooms, 0, false, &ok);
    if (!ok) return;
    try {
        uni.assignStudentToRoom(sid.toStdString(), dorm.toStdString(), room.toStdString());
        uni.logActivity("🛏", ("Assigned " + QString::fromStdString(uni.studentName(sid.toStdString()))
                       + " to Room " + room + " – " + dorm).toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void RoomsPage::reassignDialog() {
    // Pick only accommodated students
    QStringList items;
    for (const auto& s : uni.getStudents())
        if (s.isAccommodated())
            items << QString::fromStdString(
                s.getId() + " – " + s.getFullName() +
                " (Room " + s.getRoomNumber() + ", Dorm " + s.getDormitoryId() + ")");
    if (items.isEmpty()) {
        QMessageBox::warning(this, "Unavailable", "No students are currently assigned to a room.");
        return;
    }
    bool ok = false;
    QString sel = QInputDialog::getItem(this, "Reassign Student",
        "Select student to reassign:", items, 0, false, &ok);
    if (!ok) return;
    QString sid = sel.section(" – ", 0, 0);

    // Pick new dormitory
    QStringList dorms;
    for (const auto& d : uni.getDormitories())
        dorms << QString::fromStdString(d.getId());
    QString dorm = QInputDialog::getItem(this, "Reassign Student",
        "New dormitory:", dorms, 0, false, &ok);
    if (!ok) return;

    // Pick new room (only available ones)
    QStringList rooms;
    if (auto* d = uni.findDormitory(dorm.toStdString()))
        for (const auto& r : d->getRooms())
            if (!r.isFull() && !r.isUnderMaintenance())
                rooms << QString::fromStdString(r.getNumber());
    if (rooms.isEmpty()) {
        QMessageBox::warning(this, "Unavailable", "No available rooms in this dormitory.");
        return;
    }
    QString room = QInputDialog::getItem(this, "Reassign Student",
        "New room:", rooms, 0, false, &ok);
    if (!ok) return;

    try {
        uni.reassignStudent(sid.toStdString(), dorm.toStdString(), room.toStdString());
        uni.logActivity("🔄", ("Reassigned " +
            QString::fromStdString(uni.studentName(sid.toStdString()))
            + " → Room " + room + " – " + dorm).toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void RoomsPage::unassignDialog() {
    // Pick only accommodated students
    QStringList items;
    for (const auto& s : uni.getStudents())
        if (s.isAccommodated())
            items << QString::fromStdString(
                s.getId() + " – " + s.getFullName() +
                " (Room " + s.getRoomNumber() + ", Dorm " + s.getDormitoryId() + ")");
    if (items.isEmpty()) {
        QMessageBox::warning(this, "Unavailable", "No students are currently assigned to a room.");
        return;
    }
    bool ok = false;
    QString sel = QInputDialog::getItem(this, "Remove from Room",
        "Select student to remove:", items, 0, false, &ok);
    if (!ok) return;
    QString sid = sel.section(" – ", 0, 0);

    QString name = QString::fromStdString(uni.studentName(sid.toStdString()));
    if (QMessageBox::question(this, "Remove from Room",
            QString("Remove %1 from their current room?\nThey will become unassigned.").arg(name))
        != QMessageBox::Yes) return;

    try {
        uni.removeStudentFromRoom(sid.toStdString());
        uni.logActivity("🚪", ("Removed " + name + " from room").toStdString());
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

void StudentsPage::applyFilters(QTableWidget* table, QLineEdit* idEdit, QLineEdit* nameEdit, 
                                QComboBox* dormBox, QComboBox* yearBox) {
    QString sId = idEdit->text().trimmed().toLower();
    QString sName = nameEdit->text().trimmed().toLower();
    QString sDorm = dormBox->currentIndex() == 0 ? "" : dormBox->currentText();
    QString sYear = yearBox->currentIndex() == 0 ? "" : yearBox->currentText().replace("Year ", "");

    fStudentId = idEdit->text();
    fName = nameEdit->text();
    fDormIdx = dormBox->currentIndex();
    fYearIdx = yearBox->currentIndex();

    int visible = 0;
    for (int r = 0; r < table->rowCount(); ++r) {
        bool show = true;
        // ID filter - column 0
        if (!sId.isEmpty() && table->item(r, 0)) {
            if (!table->item(r, 0)->text().toLower().contains(sId)) show = false;
        }
        // Name filter - column 1
        if (show && !sName.isEmpty() && table->item(r, 1)) {
            if (!table->item(r, 1)->text().toLower().contains(sName)) show = false;
        }
        // Year filter - column 2
        if (show && !sYear.isEmpty() && table->item(r, 2)) {
            if (!table->item(r, 2)->text().contains(sYear)) show = false;
        }
        // Dormitory filter - column 4 (hidden dorm column)
        if (show && !sDorm.isEmpty() && table->item(r, 4)) {
            if (table->item(r, 4)->text() != sDorm) show = false;
        }

        table->setRowHidden(r, !show);
        if (show) ++visible;
    }

    if (auto* countLbl = table->property("countLabel").value<QLabel*>()) {
        countLbl->setText(QString("Showing %1 of %2 students").arg(visible).arg(table->rowCount()));
    }
}

void StudentsPage::refresh() {
    clearLayout(root);

    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Students",
        "Manage student records and accommodation."));
    head->addStretch();
    
    auto* rmBtn   = new QPushButton("Remove Selected");
    auto* editBtn = new QPushButton("✏️  Edit Student");
    auto* addBtn  = new QPushButton("+ Add Student");
    addBtn->setObjectName("Primary");

    editBtn->setStyleSheet(
        "QPushButton { background:#EEF1FF; color:#4C6FFF; border:1px solid #D1D5DB;"
        "border-radius:8px; padding:7px 16px; font-weight:500; }"
        "QPushButton:hover { background:#DDE3FF; }");

    head->addWidget(rmBtn); 
    head->addWidget(editBtn); 
    head->addWidget(addBtn);
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

    // Filter bar
    auto* filterCard = makeCard();
    auto* filterLay = new QVBoxLayout(filterCard);
    filterLay->setContentsMargins(20, 16, 20, 16);
    filterLay->setSpacing(12);

    auto* filterHeader = new QHBoxLayout;
    auto* filterIcon = new QLabel("🔍");
    filterIcon->setStyleSheet("font-size:14px;");
    auto* filterTitle = new QLabel("Filters");
    filterTitle->setStyleSheet("font-size:14px; font-weight:600; color:#374151;");
    filterHeader->addWidget(filterIcon);
    filterHeader->addWidget(filterTitle);
    filterHeader->addStretch();
    auto* clearBtn = new QPushButton("Clear Filters");
    clearBtn->setStyleSheet(
        "QPushButton { background:transparent; border:none; color:#EF4444;"
        "font-size:12px; font-weight:500; padding:0; }"
        "QPushButton:hover { color:#DC2626; }");
    clearBtn->setCursor(Qt::PointingHandCursor);
    filterHeader->addWidget(clearBtn);
    filterLay->addLayout(filterHeader);

    auto* filterRow = new QHBoxLayout;
    filterRow->setSpacing(12);

    auto* idEdit = new QLineEdit;
    idEdit->setPlaceholderText("Search by ID...");
    idEdit->setMinimumWidth(100);
    idEdit->setText(fStudentId);

    auto* nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("Search by Name...");
    nameEdit->setMinimumWidth(140);
    nameEdit->setText(fName);

    auto* dormCombo = new QComboBox;
    dormCombo->addItem("All Dormitories");
    for (const auto& d : uni.getDormitories())
        dormCombo->addItem(QString::fromStdString(d.getName()));
    dormCombo->setMinimumWidth(140);
    if (fDormIdx < dormCombo->count()) dormCombo->setCurrentIndex(fDormIdx);

    auto* yearCombo = new QComboBox;
    yearCombo->addItems({"All Years", "Year 1", "Year 2", "Year 3", "Year 4", "Year 5"});
    yearCombo->setMinimumWidth(110);
    if (fYearIdx < yearCombo->count()) yearCombo->setCurrentIndex(fYearIdx);

    filterRow->addWidget(idEdit);
    filterRow->addWidget(nameEdit);
    filterRow->addWidget(dormCombo);
    filterRow->addWidget(yearCombo);
    filterLay->addLayout(filterRow);
    root->addWidget(filterCard);

    auto* countLbl = new QLabel;
    countLbl->setStyleSheet("font-size:12px; color:#6B7280; padding:0 2px;");
    root->addWidget(countLbl);

    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);
    auto* table = makeTable({"ID", "Full Name", "Year", "Accommodation", "HiddenDorm"});
    table->setColumnHidden(4, true);

    table->setProperty("countLabel", QVariant::fromValue(countLbl));

    auto& studs = uni.getStudents();
    table->setRowCount((int)studs.size());
    table->setMinimumHeight(300);
    for (int i = 0; i < (int)studs.size(); ++i) {
        const auto& s = studs[i];
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(s.getId())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(s.getFullName())));
        table->setItem(i, 2, new QTableWidgetItem(QString("Year %1").arg(s.getAcademicYear())));
        table->setCellWidget(i, 3, pill(
            QString::fromStdString(s.accommodationStatus()),
            s.accommodationStatus() == "Not assigned" ? "Maintenance" : "Available"));
            
        QString dormName = "";
        if (auto* d = uni.findDormitory(s.getDormitoryId())) {
            dormName = QString::fromStdString(d->getName());
        }
        table->setItem(i, 4, new QTableWidgetItem(dormName));
    }
    tv->addWidget(table);
    root->addWidget(tableCard);

    auto doFilter = [this, table, idEdit, nameEdit, dormCombo, yearCombo]{
        applyFilters(table, idEdit, nameEdit, dormCombo, yearCombo);
    };
    connect(idEdit, &QLineEdit::textChanged, this, doFilter);
    connect(nameEdit, &QLineEdit::textChanged, this, doFilter);
    connect(dormCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, doFilter);
    connect(yearCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, doFilter);

    connect(clearBtn, &QPushButton::clicked, this, [=]{
        idEdit->clear();
        nameEdit->clear();
        dormCombo->setCurrentIndex(0);
        yearCombo->setCurrentIndex(0);
    });

    doFilter();

    connect(addBtn, &QPushButton::clicked, this, [this]{ addDialog(); });
    
    connect(editBtn, &QPushButton::clicked, this, [this, table]{
        int r = table->currentRow();
        if (r < 0) {
            QMessageBox::information(this, "Edit", "Select a student row first.");
            return;
        }
        QString id = table->item(r, 0)->text();
        
        Student* s = uni.findStudent(id.toStdString());
        if (!s) return;
        
        bool ok = false;
        QString name = QString::fromStdString(s->getFullName());
        while (true) {
            name = QInputDialog::getText(this, "Edit Student",
                "Full name (first and last name):",
                QLineEdit::Normal, name, &ok);
            if (!ok) return;
            QString err = Validate::studentName(name);
            if (err.isEmpty()) break;
            QMessageBox::warning(this, "Invalid Name", err);
        }
        
        int year = QInputDialog::getInt(this, "Edit Student", "Academic year (1-5):",
                                        s->getAcademicYear(), 1, 5, 1, &ok);
        if (!ok) return;
        
        try {
            s->setFullName(name.trimmed().toStdString());
            s->setAcademicYear(year);
            uni.logActivity("✏️", "Updated student " + id.toStdString());
            refresh();
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what());
        }
    });

    connect(rmBtn, &QPushButton::clicked, this, [this, table]{
        int r = table->currentRow();
        if (r < 0) {
            QMessageBox::information(this, "Remove", "Select a student row first.");
            return;
        }
        QString id = table->item(r, 0)->text();
        try { uni.removeStudent(id.toStdString());
            uni.logActivity("👥", "Removed student " + id.toStdString());
            refresh(); }
        catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what());
        }
    });
}

void StudentsPage::addDialog() {
    bool ok = false;
    QString id;
    while (true) {
        id = QInputDialog::getText(this, "Add Student",
            "Student ID (digits only, e.g. 006):",
            QLineEdit::Normal, id, &ok);
        if (!ok) return;
        QString err = Validate::studentId(id);
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid Student ID", err);
    }
    QString name;
    while (true) {
        name = QInputDialog::getText(this, "Add Student",
            "Full name (first and last name):",
            QLineEdit::Normal, name, &ok);
        if (!ok) return;
        QString err = Validate::studentName(name);
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid Name", err);
    }
    int year = QInputDialog::getInt(this, "Add Student", "Academic year (1-5):",
                                    1, 1, 5, 1, &ok);
    if (!ok) return;
    try {
        uni.addStudent(Student(id.trimmed().toStdString(), name.trimmed().toStdString(), year));
        uni.logActivity("👥", "Added student " + name.trimmed().toStdString());
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
    
    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Weekly Menu",
        "Manage the centralized weekly menu and track daily meals served."));
    head->addStretch();
    auto* saveBtn = new QPushButton("💾 Save Changes");
    saveBtn->setObjectName("Primary");
    saveBtn->setMinimumWidth(150);
    saveBtn->setMinimumHeight(40);
    saveBtn->setStyleSheet(
        "QPushButton { background:#4C6FFF; color:white; border-radius:6px; font-weight:600; font-size:14px; }"
        "QPushButton:hover { background:#3B5BDB; }"
    );
    head->addWidget(saveBtn);
    root->addLayout(head);

    int totalMeals = 0;
    for (int i = 0; i < 7; ++i) {
        totalMeals += uni.getWeeklyMenu().days[i].mealsServed;
    }

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("🍽", QString::number(totalMeals),
                             "Total Meals", "This Week", "#FCE7F3"), 1);
    sRow->addWidget(statCard("📅", "7", "Days Planned", "Full Schedule", "#EEF1FF"), 1);
    sRow->addWidget(statCard("👥", QString::number(uni.totalStudents()), "Eligible Students", "For meals", "#DCFCE7"), 1);
    sRow->addStretch(1);
    root->addLayout(sRow);

    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);

    auto* table = makeTable({"Day", "Breakfast", "Lunch", "Dinner", "Meals Served"});
    table->setRowCount(7);
    table->setMinimumHeight(450);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(55); // Make fields big
    table->setStyleSheet(
        "QTableWidget { border: none; background: transparent; }"
        "QTableWidget::item { padding: 4px; }"
        "QLineEdit, QSpinBox { padding: 8px 12px; font-size: 14px; color: #374151; background: #F9FAFB; border: 1px solid #E5E7EB; border-radius: 6px; }"
        "QLineEdit:focus, QSpinBox:focus { background: #FFFFFF; border: 1px solid #4C6FFF; }"
    );

    const char* days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    
    for (int i = 0; i < 7; ++i) {
        const auto& d = uni.getWeeklyMenu().days[i];
        
        auto* dayItem = new QTableWidgetItem(days[i]);
        dayItem->setTextAlignment(Qt::AlignCenter);
        dayItem->setFont(QFont("Inter", 11, QFont::DemiBold));
        table->setItem(i, 0, dayItem);
        
        auto* bfE = new QLineEdit(QString::fromStdString(d.breakfast));
        table->setCellWidget(i, 1, bfE);
        
        auto* lunE = new QLineEdit(QString::fromStdString(d.lunch));
        table->setCellWidget(i, 2, lunE);
        
        auto* dinE = new QLineEdit(QString::fromStdString(d.dinner));
        table->setCellWidget(i, 3, dinE);

        auto* servedE = new QSpinBox;
        servedE->setRange(0, 100000);
        servedE->setValue(d.mealsServed);
        table->setCellWidget(i, 4, servedE);
    }

    tv->addWidget(table);
    root->addWidget(tableCard);

    connect(saveBtn, &QPushButton::clicked, this, [this, table]{
        saveMenu(table);
    });
}

void RestaurantPage::saveMenu(QTableWidget* table) {
    // Validate all fields before saving
    const char* days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    for (int i = 0; i < 7; ++i) {
        auto* bfE = qobject_cast<QLineEdit*>(table->cellWidget(i, 1));
        auto* lunE = qobject_cast<QLineEdit*>(table->cellWidget(i, 2));
        auto* dinE = qobject_cast<QLineEdit*>(table->cellWidget(i, 3));
        if (bfE) {
            QString err = Validate::menuItem(bfE->text(), QString("%1 Breakfast").arg(days[i]));
            if (!err.isEmpty()) { QMessageBox::warning(this, "Invalid Menu", err); return; }
        }
        if (lunE) {
            QString err = Validate::menuItem(lunE->text(), QString("%1 Lunch").arg(days[i]));
            if (!err.isEmpty()) { QMessageBox::warning(this, "Invalid Menu", err); return; }
        }
        if (dinE) {
            QString err = Validate::menuItem(dinE->text(), QString("%1 Dinner").arg(days[i]));
            if (!err.isEmpty()) { QMessageBox::warning(this, "Invalid Menu", err); return; }
        }
    }
    for (int i = 0; i < 7; ++i) {
        auto* bfE = qobject_cast<QLineEdit*>(table->cellWidget(i, 1));
        auto* lunE = qobject_cast<QLineEdit*>(table->cellWidget(i, 2));
        auto* dinE = qobject_cast<QLineEdit*>(table->cellWidget(i, 3));
        auto* servedE = qobject_cast<QSpinBox*>(table->cellWidget(i, 4));
        
        if (bfE && lunE && dinE && servedE) {
            uni.getWeeklyMenu().days[i].breakfast = bfE->text().trimmed().toStdString();
            uni.getWeeklyMenu().days[i].lunch = lunE->text().trimmed().toStdString();
            uni.getWeeklyMenu().days[i].dinner = dinE->text().trimmed().toStdString();
            uni.getWeeklyMenu().days[i].mealsServed = servedE->value();
        }
    }
    uni.logActivity("💾", "Updated the weekly menu");
    QMessageBox::information(this, "Saved", "Weekly menu saved successfully.");
    refresh();
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

    auto* sRow = new QHBoxLayout; sRow->setSpacing(20);
    
    // Stat Card
    auto* totalCard = statCard("📅", QString::number((int)uni.getBookings().size()),
                               "Total Bookings", "All time", "#EEF1FF");
    totalCard->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sRow->addWidget(totalCard);

    // Filters container (vertically centered next to the card)
    auto* controlsLayout = new QVBoxLayout;
    controlsLayout->setAlignment(Qt::AlignVCenter);
    
    auto* controlsH = new QHBoxLayout;
    controlsH->setSpacing(12);

    auto* searchBar = new QLineEdit;
    searchBar->setPlaceholderText("Search by student name...");
    searchBar->setMinimumWidth(260);
    searchBar->setStyleSheet("padding: 10px 14px; font-size: 14px; border: 1px solid #D1D5DB; border-radius: 8px; background: white;");
    
    auto* mealCombo = new QComboBox;
    mealCombo->addItems({"All Meals", "Breakfast", "Lunch", "Dinner"});
    mealCombo->setMinimumWidth(160);
    mealCombo->setStyleSheet("padding: 10px 14px; font-size: 14px; border: 1px solid #D1D5DB; border-radius: 8px; background: white;");

    controlsH->addWidget(searchBar);
    controlsH->addWidget(mealCombo);
    controlsH->addStretch();
    
    controlsLayout->addLayout(controlsH);
    sRow->addLayout(controlsLayout);
    sRow->addStretch();

    root->addLayout(sRow);

    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);
    auto* table = makeTable({"Student", "Date", "Meal"});
    auto& bk = uni.getBookings();
    table->setRowCount((int)bk.size());
    table->setMinimumHeight(300);
    for (int i = 0; i < (int)bk.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(uni.studentName(bk[i].studentId))));
        table->setItem(i, 1, new QTableWidgetItem(
            Validate::toDisplay(bk[i].date)));
        table->setItem(i, 2, new QTableWidgetItem(
            QString::fromStdString(mealTypeToString(bk[i].meal))));
    }
    tv->addWidget(table);
    root->addWidget(tableCard);

    auto doFilter = [table, searchBar, mealCombo](){
        QString filter = searchBar->text().trimmed().toLower();
        QString mealFilter = mealCombo->currentText();
        bool filterMeal = (mealCombo->currentIndex() > 0);
        for (int i = 0; i < table->rowCount(); ++i) {
            bool matchName = table->item(i, 0)->text().toLower().contains(filter);
            bool matchMeal = !filterMeal || table->item(i, 2)->text() == mealFilter;
            table->setRowHidden(i, !(matchName && matchMeal));
        }
    };

    connect(searchBar, &QLineEdit::textChanged, this, doFilter);
    connect(mealCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, doFilter);

    connect(btn, &QPushButton::clicked, this, [this]{ bookDialog(); });
}

void MealBookingPage::bookDialog() {
    QString sid = pickStudentId(this, uni);
    if (sid.isEmpty()) return;
    bool ok = false;
    QString date;
    QDate parsedDate;
    while (true) {
        date = QInputDialog::getText(this, "Book Meal",
            "Date (dd-mm-yyyy):",
            QLineEdit::Normal,
            date.isEmpty() ? Validate::todayUserFormat() : date, &ok);
        if (!ok) return;
        QString err = Validate::futureDate(date, &parsedDate);
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid Date", err);
    }
    QString meal = QInputDialog::getItem(this, "Book Meal", "Meal:",
                                         {"Breakfast", "Lunch", "Dinner"}, 1, false, &ok);
    if (!ok) return;
    try {
        QString internalDate = Validate::toInternal(date);
        uni.bookMeal(MealBooking(sid.toStdString(), internalDate.toStdString(),
                                  mealTypeFromString(meal.toStdString())));
        uni.logActivity("📅", ("Booked " + meal + " for "
                       + QString::fromStdString(uni.studentName(sid.toStdString()))).toStdString());
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
            Validate::toDisplay(ap[i].date)));
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

    // Date validation
    QString date;
    QDate parsedDate;
    while (true) {
        date = QInputDialog::getText(this, "Schedule Appointment",
            "Date (dd-mm-yyyy):",
            QLineEdit::Normal,
            date.isEmpty() ? Validate::todayUserFormat() : date, &ok);
        if (!ok) return;
        QString err = Validate::futureDate(date, &parsedDate);
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid Date", err);
    }

    // Time validation
    QString time;
    while (true) {
        time = QInputDialog::getText(this, "Schedule Appointment",
            "Time (HH:MM, 24-hour format):",
            QLineEdit::Normal,
            time.isEmpty() ? "10:00" : time, &ok);
        if (!ok) return;
        QString err = Validate::time(time);
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid Time", err);
    }

    // Reason validation
    QString reason;
    while (true) {
        reason = QInputDialog::getText(this, "Schedule Appointment",
            "Reason for appointment:",
            QLineEdit::Normal,
            reason.isEmpty() ? "Check-up" : reason, &ok);
        if (!ok) return;
        QString err = Validate::notEmpty(reason, "Reason");
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid Reason", err);
    }

    try {
        QString internalDate = Validate::toInternal(date);
        uni.getClinic().schedule(
            Appointment(sid.toStdString(), internalDate.toStdString(),
                        time.trimmed().toStdString(), reason.trimmed().toStdString()));
        uni.logActivity("🏥", ("Scheduled appointment for "
                       + QString::fromStdString(uni.studentName(sid.toStdString()))).toStdString());
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
    auto* addBtn    = new QPushButton("+ Add Activity");
    addBtn->setObjectName("Primary");
    head->addWidget(addBtn);
    root->addLayout(head);

    // Count
    int count = 0, approved = 0, pending = 0;
    for (const auto& a : uni.getActivities()) {
        if (QString::fromStdString(a->category()) != category) continue;
        ++count;
        for (const auto& e : a->getEnrollments()) {
            if (e.status == "Approved") ++approved;
            else if (e.status == "Pending") ++pending;
        }
    }

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard(sports ? "⚽" : "🎭",
        QString::number(count), "Activities", "Registered",
        sports ? "#F0FDF4" : "#FFF7ED"));
    sRow->addWidget(statCard("✅", QString::number(approved),
                             "Approved", "Enrolled", "#DCFCE7"));
    sRow->addWidget(statCard("⏳", QString::number(pending),
                             "Pending", "Awaiting review", "#FEF3C7", "#D97706"));
    root->addLayout(sRow);

    // Activity cards with enrollment details
    for (const auto& a : uni.getActivities()) {
        if (QString::fromStdString(a->category()) != category) continue;

        auto* card = makeCard();
        auto* v = new QVBoxLayout(card);
        v->setContentsMargins(24, 20, 24, 20); v->setSpacing(12);

        auto* hr = new QHBoxLayout;
        auto* ic = new QLabel(sports ? "⚽" : "🎭");
        ic->setFixedSize(44, 44); ic->setAlignment(Qt::AlignCenter);
        ic->setStyleSheet(QString("background:") +
                          (sports ? "#F0FDF4" : "#FFF7ED") +
                          "; border-radius:12px; font-size:20px;");
        auto* aName = new QLabel(QString::fromStdString(a->getName()));
        aName->setStyleSheet("font-size:16px; font-weight:700;");
        hr->addWidget(ic); hr->addWidget(aName, 1);
        v->addLayout(hr);

        auto* desc = new QLabel(QString::fromStdString(a->describe()));
        desc->setStyleSheet("font-size:12px; color:#6B7280;");
        desc->setWordWrap(true);
        v->addWidget(desc);

        // Enrollments list
        const auto& enrollments = a->getEnrollments();
        if (!enrollments.empty()) {
            v->addWidget(hDivider());
            auto* enrollTitle = new QLabel("📋  Applications & Enrollments");
            enrollTitle->setStyleSheet("font-size:13px; font-weight:700; color:#374151;");
            v->addWidget(enrollTitle);

            for (const auto& e : enrollments) {
                auto* row = new QHBoxLayout;
                row->setSpacing(10);

                auto* nameLabel = new QLabel("👤 " + QString::fromStdString(uni.studentName(e.studentId)));
                nameLabel->setStyleSheet("font-size:13px; font-weight:500; color:#111827;");
                row->addWidget(nameLabel, 1);

                if (e.status == "Pending") {
                    auto* statusLabel = new QLabel("⏳ Pending");
                    statusLabel->setStyleSheet("font-size:12px; font-weight:600; color:#D97706; padding:4px 10px;"
                        " background:#FEF3C7; border-radius:8px;");
                    row->addWidget(statusLabel);

                    auto* approveBtn = new QPushButton("✅ Approve");
                    approveBtn->setCursor(Qt::PointingHandCursor);
                    approveBtn->setStyleSheet(
                        "QPushButton { background:#DCFCE7; color:#16A34A; border:none; border-radius:8px;"
                        " padding:6px 14px; font-size:12px; font-weight:600; }"
                        "QPushButton:hover { background:#BBF7D0; }");

                    auto* refuseBtn = new QPushButton("❌ Refuse");
                    refuseBtn->setCursor(Qt::PointingHandCursor);
                    refuseBtn->setStyleSheet(
                        "QPushButton { background:#FEE2E2; color:#EF4444; border:none; border-radius:8px;"
                        " padding:6px 14px; font-size:12px; font-weight:600; }"
                        "QPushButton:hover { background:#FECACA; }");

                    row->addWidget(approveBtn);
                    row->addWidget(refuseBtn);

                    QString actName = QString::fromStdString(a->getName());
                    QString sid = QString::fromStdString(e.studentId);
                    connect(approveBtn, &QPushButton::clicked, this, [this, actName, sid]() {
                        for (auto& act : uni.getActivities()) {
                            if (QString::fromStdString(act->getName()) == actName) {
                                act->approve(sid.toStdString());
                                uni.logActivity("✅", "Approved " +
                                    uni.studentName(sid.toStdString()) + " for " + actName.toStdString());
                                break;
                            }
                        }
                        refresh();
                    });
                    connect(refuseBtn, &QPushButton::clicked, this, [this, actName, sid]() {
                        for (auto& act : uni.getActivities()) {
                            if (QString::fromStdString(act->getName()) == actName) {
                                act->refuse(sid.toStdString());
                                uni.logActivity("❌", "Refused " +
                                    uni.studentName(sid.toStdString()) + " for " + actName.toStdString());
                                break;
                            }
                        }
                        refresh();
                    });
                } else if (e.status == "Approved") {
                    auto* statusLabel = new QLabel("✅ Approved");
                    statusLabel->setStyleSheet("font-size:12px; font-weight:600; color:#16A34A; padding:4px 10px;"
                        " background:#DCFCE7; border-radius:8px;");
                    row->addWidget(statusLabel);
                } else {
                    auto* statusLabel = new QLabel("❌ Refused");
                    statusLabel->setStyleSheet("font-size:12px; font-weight:600; color:#EF4444; padding:4px 10px;"
                        " background:#FEE2E2; border-radius:8px;");
                    row->addWidget(statusLabel);
                }

                v->addLayout(row);
            }
        } else {
            auto* noEnroll = new QLabel("No applications yet.");
            noEnroll->setStyleSheet("font-size:12px; color:#9CA3AF; font-style:italic;");
            v->addWidget(noEnroll);
        }

        root->addWidget(card);
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
    }
    root->addStretch();

    connect(addBtn, &QPushButton::clicked, this, [this]{ addDialog(); });
}

void ActivityPage::addDialog() {
    const bool sports = (category == "Sports");
    bool ok = false;

    // Activity name validation
    QString name;
    while (true) {
        name = QInputDialog::getText(this, "Add Activity", "Activity name:",
            QLineEdit::Normal, name, &ok);
        if (!ok) return;
        QString err = Validate::activityName(name);
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid Activity Name", err);
    }

    // Schedule validation
    QString sched;
    while (true) {
        sched = QInputDialog::getText(this, "Add Activity",
            "Schedule (e.g. Mon & Wed 18:00):",
            QLineEdit::Normal, sched, &ok);
        if (!ok) return;
        QString err = Validate::notEmpty(sched, "Schedule");
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid Schedule", err);
    }

    // Extra info validation
    QString extraLabel = sports ? "Coach" : "Venue";
    QString extra;
    while (true) {
        extra = QInputDialog::getText(this, "Add Activity",
            extraLabel + ":", QLineEdit::Normal, extra, &ok);
        if (!ok) return;
        QString err = Validate::notEmpty(extra, extraLabel);
        if (err.isEmpty()) break;
        QMessageBox::warning(this, "Invalid " + extraLabel, err);
    }

    if (sports)
        uni.addActivity(std::make_unique<SportsActivity>(
            name.trimmed().toStdString(), sched.trimmed().toStdString(), extra.trimmed().toStdString()));
    else
        uni.addActivity(std::make_unique<CulturalActivity>(
            name.trimmed().toStdString(), sched.trimmed().toStdString(), extra.trimmed().toStdString()));
    uni.logActivity(sports ? "⚽" : "🎭", ("Added activity " + name.trimmed()).toStdString());
    refresh();
}

void ActivityPage::enrollDialog() {
    // No longer needed — enrollment happens through student applications
}
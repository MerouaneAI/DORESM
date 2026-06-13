#include "gui/DormAdminPages.h"
#include "gui/widgets.h"
#include "model/University.h"
#include "model/Activity.h"
#include "model/WeeklyMenu.h"
#include "model/MealBooking.h"

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
#include <QDate>
#include <QComboBox>
#include <algorithm>

// ─── Helpers ─────────────────────────────────────────────────────────────────

namespace {

void clearDormLayout(QLayout* layout) {
    if (!layout) return;
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        if (item->layout()) clearDormLayout(item->layout());
        delete item;
    }
}

QTableWidget* makeDormTable(const QStringList& cols) {
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

QProgressBar* makeDormOccupBar(int pct, const QString& colorHex) {
    auto* bar = new QProgressBar;
    bar->setValue(pct);
    bar->setTextVisible(false);
    bar->setFixedHeight(7);
    bar->setStyleSheet(
        "QProgressBar { background:#F1F2F4; border-radius:4px; border:none; }"
        "QProgressBar::chunk { background:" + colorHex + "; border-radius:4px; }");
    return bar;
}

QFrame* dormHDivider() {
    auto* f = new QFrame;
    f->setFrameShape(QFrame::HLine);
    f->setStyleSheet("color:#F3F4F8;");
    return f;
}

// Helper to check if a student belongs to a given dormitory
bool studentInDorm(const University& uni, const std::string& studentId, const std::string& dormId) {
    for (const auto& s : uni.getStudents()) {
        if (s.getId() == studentId && s.getDormitoryId() == dormId)
            return true;
    }
    return false;
}

// Pick a student who is in this dormitory
QString pickDormStudentId(QWidget* p, University& uni, const std::string& dormId) {
    QStringList items;
    for (const auto& s : uni.getStudents()) {
        if (s.getDormitoryId() == dormId)
            items << QString::fromStdString(s.getId() + " – " + s.getFullName());
    }
    if (items.isEmpty()) {
        QMessageBox::warning(p, "Unavailable", "No students in this dormitory.");
        return {};
    }
    bool ok = false;
    QString sel = QInputDialog::getItem(p, "Select Student", "Student:", items, 0, false, &ok);
    if (!ok) return {};
    return sel.section(" – ", 0, 0);
}

} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
//  DormAdmin Dashboard
// ═══════════════════════════════════════════════════════════════════════════════

DormAdminDashboardPage::DormAdminDashboardPage(University& u, const std::string& did,
                                               QWidget* parent)
    : QWidget(parent), uni(u), dormId(did)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void DormAdminDashboardPage::setNavigator(std::function<void(int)> nav) {
    navigate = std::move(nav);
}

void DormAdminDashboardPage::refresh() {
    clearDormLayout(root);

    Dormitory* d = uni.findDormitory(dormId);
    if (!d) { root->addWidget(new QLabel("Dormitory not found.")); return; }
    QString dormName = QString::fromStdString(d->getName());

    // Header
    root->addWidget(pageHeader("Welcome, " + dormName + " Admin",
                               "Overview of your dormitory's status and operations."));

    // ── Stat row ──────────────────────────────────────────────────────
    int cap = d->totalCapacity();
    int occ = d->totalOccupancy();
    int avail = d->availableRoomCount();
    int totalRooms = (int)d->getRooms().size();
    int maintRooms = 0;
    for (const auto& r : d->getRooms())
        if (r.isUnderMaintenance()) ++maintRooms;

    auto* statRow = new QHBoxLayout;
    statRow->setSpacing(16);
    statRow->addWidget(statCard("👥", QString::number(occ),
                                "Residents", QString("of %1 capacity").arg(cap), "#EEF1FF"));
    statRow->addWidget(statCard("🛏", QString::number(totalRooms),
                                "Total Rooms", QString("%1 available").arg(avail), "#DCFCE7"));
    statRow->addWidget(statCard("✅", QString::number(avail),
                                "Available Rooms", "Ready to assign", "#FEF3C7", "#D97706"));
    statRow->addWidget(statCard("🔧", QString::number(maintRooms),
                                "Maintenance", "Under repair", "#FCE7F3", "#DB2777"));
    root->addLayout(statRow);

    // ── Quick Access Modules ──────────────────────────────────────────
    auto* modLabel = new QLabel("Quick Access");
    modLabel->setObjectName("SectionTitle");
    root->addWidget(modLabel);

    auto* modGrid = new QGridLayout;
    modGrid->setSpacing(16);

    struct Mod { const char* emoji; const char* title; const char* desc; const char* bg; int target; };
    Mod mods[] = {
        {"🏢", "Dormitory",     "View your dormitory details and manage rooms.",         "#EEF1FF", 1},
        {"🛏", "Room Management", "Assign, reassign, and maintain rooms in your dorm.",   "#E0F2FE", 2},
        {"👥", "Students",       "View students assigned to your dormitory.",             "#DCFCE7", 3},
        {"🍽", "Restaurant",     "View the weekly menu for your dorm residents.",         "#FCE7F3", 4},
        {"📅", "Meal Booking",   "View meal bookings by your dorm residents.",            "#FEF3C7", 5},
        {"🏥", "Health Clinic",  "View health appointments for your dorm residents.",     "#FFF7ED", 8},
    };

    for (int i = 0; i < 6; ++i) {
        int target = mods[i].target;
        auto* mc = moduleCard(mods[i].emoji, mods[i].title,
                              mods[i].desc, mods[i].bg,
                              [this, target]{ if (navigate) navigate(target); });
        mc->setMinimumHeight(150);
        modGrid->addWidget(mc, i / 3, i % 3);
    }
    root->addLayout(modGrid);

    // ── Occupancy bar ─────────────────────────────────────────────────
    auto* occCard = makeCard();
    auto* occV = new QVBoxLayout(occCard);
    occV->setContentsMargins(24, 20, 24, 20);
    occV->setSpacing(10);

    auto* occTitle = new QLabel("Dormitory Occupancy");
    occTitle->setStyleSheet("font-size:15px; font-weight:700; color:#111827;");
    occV->addWidget(occTitle);

    int pct = cap ? occ * 100 / cap : 0;
    auto* pctRow = new QHBoxLayout;
    auto* pctLbl = new QLabel(QString("%1% occupied  ·  %2/%3 beds").arg(pct).arg(occ).arg(cap));
    pctLbl->setStyleSheet("font-size:13px; color:#6B7280;");
    pctRow->addWidget(pctLbl);
    pctRow->addStretch();
    occV->addLayout(pctRow);
    occV->addWidget(makeDormOccupBar(pct, pct >= 90 ? "#EF4444" : pct >= 50 ? "#4C6FFF" : "#10B981"));
    root->addWidget(occCard);

    root->addStretch();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  DormAdmin Dormitory Page (single-dorm view)
// ═══════════════════════════════════════════════════════════════════════════════

DormAdminDormitoryPage::DormAdminDormitoryPage(University& u, const std::string& did,
                                               QWidget* parent)
    : QWidget(parent), uni(u), dormId(did)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void DormAdminDormitoryPage::refresh() {
    clearDormLayout(root);

    Dormitory* d = uni.findDormitory(dormId);
    if (!d) { root->addWidget(new QLabel("Dormitory not found.")); return; }
    QString dormName = QString::fromStdString(d->getName());

    // Header with action buttons
    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("🏢  " + dormName, "Manage rooms in your dormitory."));
    head->addStretch();

    auto* addRoomBtn = new QPushButton("+ Add Room");
    addRoomBtn->setObjectName("Primary");
    auto* delRoomBtn = new QPushButton("Delete Room");
    head->addWidget(delRoomBtn);
    head->addWidget(addRoomBtn);
    root->addLayout(head);

    // ── Stats ─────────────────────────────────────────────────────────
    int cap = d->totalCapacity(), occ = d->totalOccupancy();
    int pct = cap ? occ * 100 / cap : 0;

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("🛏", QString::number((int)d->getRooms().size()),
                             "Total Rooms", "In dormitory", "#EEF1FF"));
    sRow->addWidget(statCard("👥", QString::number(occ),
                             "Occupied Beds", "Current residents", "#DCFCE7"));
    sRow->addWidget(statCard("✅", QString::number(cap - occ),
                             "Available Beds", "Ready to assign", "#FEF3C7"));
    sRow->addWidget(statCard("📊", QString::number(pct) + "%",
                             "Occupancy Rate", "", "#FCE7F3"));
    root->addLayout(sRow);

    // ── Room list ─────────────────────────────────────────────────────
    auto* roomLabel = new QLabel("All Rooms");
    roomLabel->setObjectName("SectionTitle");
    root->addWidget(roomLabel);

    auto* grid = new QGridLayout;
    grid->setSpacing(16);
    int col = 0, row = 0;

    for (const auto& rm : d->getRooms()) {
        auto* card = makeCard();
        auto* v = new QVBoxLayout(card);
        v->setContentsMargins(20, 18, 20, 18); v->setSpacing(8);

        auto* hr = new QHBoxLayout;
        auto* roomIcon = new QLabel("🛏");
        roomIcon->setFixedSize(40, 40);
        roomIcon->setAlignment(Qt::AlignCenter);
        roomIcon->setStyleSheet("background:#EEF1FF; border-radius:12px; font-size:18px;");

        auto* roomInfo = new QVBoxLayout;
        roomInfo->setSpacing(2);
        auto* roomNum = new QLabel("Room " + QString::fromStdString(rm.getNumber()));
        roomNum->setStyleSheet("font-size:15px; font-weight:700; color:#111827;");
        auto* roomType = new QLabel(QString::fromStdString(rm.getType()) +
            "  ·  " + QString::number(rm.getOccupancy()) + "/" + QString::number(rm.getCapacity()));
        roomType->setStyleSheet("font-size:12px; color:#6B7280;");
        roomInfo->addWidget(roomNum);
        roomInfo->addWidget(roomType);

        hr->addWidget(roomIcon);
        hr->addLayout(roomInfo, 1);

        QString statusText = QString::fromStdString(rm.status());
        hr->addWidget(pill(statusText, statusText));
        v->addLayout(hr);

        // Occupants
        if (!rm.getOccupants().empty()) {
            v->addWidget(dormHDivider());
            for (const auto& occ : rm.getOccupants()) {
                auto* occLbl = new QLabel("  👤  " + QString::fromStdString(uni.studentName(occ)));
                occLbl->setStyleSheet("font-size:12px; color:#374151;");
                v->addWidget(occLbl);
            }
        }

        grid->addWidget(card, row, col);
        if (++col == 3) { col = 0; ++row; }
    }
    root->addLayout(grid);

    // ── Restaurant info ───────────────────────────────────────────────
    auto* restCard = makeCard();
    auto* restLay = new QHBoxLayout(restCard);
    restLay->setContentsMargins(24, 18, 24, 18);
    restLay->setSpacing(16);

    auto* restIcon = new QLabel("🍽");
    restIcon->setFixedSize(44, 44);
    restIcon->setAlignment(Qt::AlignCenter);
    restIcon->setStyleSheet("background:#FCE7F3; border-radius:12px; font-size:20px;");
    auto* restInfo = new QVBoxLayout;
    restInfo->setSpacing(2);
    auto* restName = new QLabel(QString::fromStdString(d->getRestaurant().getName()));
    restName->setStyleSheet("font-size:15px; font-weight:600; color:#111827;");
    auto* restSub = new QLabel("Serves centralized weekly menu");
    restSub->setStyleSheet("font-size:12px; color:#6B7280;");
    restInfo->addWidget(restName);
    restInfo->addWidget(restSub);
    restLay->addWidget(restIcon);
    restLay->addLayout(restInfo, 1);
    root->addWidget(restCard);

    root->addStretch();

    // ── Connections ───────────────────────────────────────────────────
    connect(addRoomBtn, &QPushButton::clicked, this, [this]{ addRoomDialog(); });
    connect(delRoomBtn, &QPushButton::clicked, this, [this]{ deleteRoomDialog(); });
}

void DormAdminDormitoryPage::addRoomDialog() {
    Dormitory* d = uni.findDormitory(dormId);
    if (!d) return;
    bool ok = false;
    QString num = QInputDialog::getText(this, "Add Room", "Room number:",
                                        QLineEdit::Normal, "", &ok);
    if (!ok || num.trimmed().isEmpty()) return;
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

void DormAdminDormitoryPage::deleteRoomDialog() {
    Dormitory* d = uni.findDormitory(dormId);
    if (!d) return;
    QStringList rooms;
    for (const auto& r : d->getRooms()) rooms << QString::fromStdString(r.getNumber());
    if (rooms.isEmpty()) { QMessageBox::information(this, "Delete Room", "No rooms to delete."); return; }
    bool ok = false;
    QString num = QInputDialog::getItem(this, "Delete Room", "Room:", rooms, 0, false, &ok);
    if (!ok) return;
    try {
        d->removeRoom(num.toStdString());
        uni.logActivity("🗑", ("Deleted room " + num + " from " +
                               QString::fromStdString(d->getName())).toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  DormAdmin Rooms Page (full management, scoped to dorm)
// ═══════════════════════════════════════════════════════════════════════════════

DormAdminRoomsPage::DormAdminRoomsPage(University& u, const std::string& did,
                                       QWidget* parent)
    : QWidget(parent), uni(u), dormId(did)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void DormAdminRoomsPage::applyFilters(QTableWidget* table, QComboBox* typeBox,
                                      QComboBox* statusBox,
                                      QLineEdit* studentEdit, QLineEdit* roomEdit) {
    QString sId   = studentEdit->text().trimmed().toLower();
    QString sRoom = roomEdit->text().trimmed().toLower();
    QString sType = typeBox->currentIndex() == 0 ? "" : typeBox->currentText();
    QString sStat = statusBox->currentIndex() == 0 ? "" : statusBox->currentText();

    fStudentId = studentEdit->text().toStdString();
    fRoom      = roomEdit->text().toStdString();
    fTypeIdx   = typeBox->currentIndex();
    fStatusIdx = statusBox->currentIndex();

    int visible = 0;
    for (int r = 0; r < table->rowCount(); ++r) {
        bool show = true;
        if (!sId.isEmpty() && table->item(r, 5))
            if (!table->item(r, 5)->text().toLower().contains(sId)) show = false;
        if (show && !sRoom.isEmpty() && table->item(r, 0))
            if (!table->item(r, 0)->text().toLower().contains(sRoom)) show = false;
        if (show && !sType.isEmpty() && table->item(r, 1))
            if (table->item(r, 1)->text() != sType) show = false;
        if (show && !sStat.isEmpty() && table->item(r, 6))
            if (table->item(r, 6)->text() != sStat) show = false;

        table->setRowHidden(r, !show);
        if (show) ++visible;
    }

    if (auto* countLbl = table->property("countLabel").value<QLabel*>())
        countLbl->setText(QString("Showing %1 of %2 rooms").arg(visible).arg(table->rowCount()));
}

void DormAdminRoomsPage::refresh() {
    clearDormLayout(root);

    Dormitory* d = uni.findDormitory(dormId);
    if (!d) { root->addWidget(new QLabel("Dormitory not found.")); return; }
    QString dormName = QString::fromStdString(d->getName());

    // ── Header with action buttons ───────────────────────────────────
    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Room Management – " + dormName,
        "View, assign, reassign, and manage rooms in your dormitory."));
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

    // ── Stat cards ───────────────────────────────────────────────────
    int maintRooms = 0;
    int totalRooms = (int)d->getRooms().size();
    int availRooms = d->availableRoomCount();
    int occRooms = 0;
    for (const auto& r : d->getRooms()) {
        if (r.isUnderMaintenance()) ++maintRooms;
        if (!r.isEmpty() && !r.isUnderMaintenance()) ++occRooms;
    }

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("🛏",  QString::number(totalRooms),
                             "Total Rooms",    "In this dormitory", "#EEF1FF"));
    sRow->addWidget(statCard("✅",  QString::number(availRooms),
                             "Available",      "Ready to assign",  "#DCFCE7"));
    sRow->addWidget(statCard("🔴", QString::number(occRooms),
                             "Occupied",       "Currently in use", "#FEE2E2", "#EF4444"));
    sRow->addWidget(statCard("🔧", QString::number(maintRooms),
                             "Maintenance",    "Under repair",     "#FEF3C7", "#D97706"));
    root->addLayout(sRow);

    // ── Filter bar ───────────────────────────────────────────────────
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

    auto* studentIdEdit = new QLineEdit;
    studentIdEdit->setPlaceholderText("Search by Student ID...");
    studentIdEdit->setMinimumWidth(140);
    studentIdEdit->setText(QString::fromStdString(fStudentId));

    auto* roomEdit = new QLineEdit;
    roomEdit->setPlaceholderText("Room number...");
    roomEdit->setMinimumWidth(120);
    roomEdit->setText(QString::fromStdString(fRoom));

    auto* typeCombo = new QComboBox;
    typeCombo->addItems({"All Types", "Single", "Double", "Triple"});
    typeCombo->setMinimumWidth(110);
    if (fTypeIdx < typeCombo->count()) typeCombo->setCurrentIndex(fTypeIdx);

    auto* statusCombo = new QComboBox;
    statusCombo->addItems({"All Status", "Available", "Partial", "Full", "Maintenance"});
    statusCombo->setMinimumWidth(120);
    if (fStatusIdx < statusCombo->count()) statusCombo->setCurrentIndex(fStatusIdx);

    filterRow->addWidget(studentIdEdit);
    filterRow->addWidget(roomEdit);
    filterRow->addWidget(typeCombo);
    filterRow->addWidget(statusCombo);
    filterLay->addLayout(filterRow);
    root->addWidget(filterCard);

    auto* countLbl = new QLabel;
    countLbl->setStyleSheet("font-size:12px; color:#6B7280; padding:0 2px;");
    root->addWidget(countLbl);

    // ── Table ────────────────────────────────────────────────────────
    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);

    // Columns: Room, Type, Occupancy, Status, Residents, StudentIDs(hidden), RawStatus(hidden)
    auto* table = makeDormTable({"Room", "Type", "Occupancy", "Status", "Residents",
                                 "StudentIDs", "RawStatus"});
    table->setColumnHidden(5, true);
    table->setColumnHidden(6, true);
    table->setProperty("countLabel", QVariant::fromValue(countLbl));

    table->setRowCount(totalRooms);
    table->setMinimumHeight(300);

    int i = 0;
    for (const auto& rm : d->getRooms()) {
        table->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(rm.getNumber())));
        table->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(rm.getType())));
        table->setItem(i, 2, new QTableWidgetItem(
            QString("%1/%2").arg(rm.getOccupancy()).arg(rm.getCapacity())));

        QString statusText = QString::fromStdString(rm.status());
        table->setCellWidget(i, 3, pill(statusText, statusText));

        QStringList names, ids;
        for (const auto& occ : rm.getOccupants()) {
            names << QString::fromStdString(uni.studentName(occ));
            ids   << QString::fromStdString(occ);
        }
        table->setItem(i, 4, new QTableWidgetItem(
            names.isEmpty() ? "—" : names.join(", ")));
        table->setItem(i, 5, new QTableWidgetItem(ids.join(" ")));
        table->setItem(i, 6, new QTableWidgetItem(statusText));
        ++i;
    }

    tv->addWidget(table);
    root->addWidget(tableCard);

    // ── Connect filters ──────────────────────────────────────────────
    auto doFilter = [this, table, typeCombo, statusCombo, studentIdEdit, roomEdit]{
        applyFilters(table, typeCombo, statusCombo, studentIdEdit, roomEdit);
    };
    connect(studentIdEdit, &QLineEdit::textChanged, this, doFilter);
    connect(roomEdit,      &QLineEdit::textChanged, this, doFilter);
    connect(typeCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, doFilter);
    connect(statusCombo,QOverload<int>::of(&QComboBox::currentIndexChanged), this, doFilter);

    connect(clearBtn, &QPushButton::clicked, this, [=]{
        studentIdEdit->clear();
        roomEdit->clear();
        typeCombo->setCurrentIndex(0);
        statusCombo->setCurrentIndex(0);
    });

    doFilter();

    // ── Connect action buttons ───────────────────────────────────────
    connect(assignBtn,   &QPushButton::clicked, this, [this]{ assignDialog(); });
    connect(reassignBtn, &QPushButton::clicked, this, [this]{ reassignDialog(); });
    connect(unassignBtn, &QPushButton::clicked, this, [this]{ unassignDialog(); });
    connect(maintBtn,    &QPushButton::clicked, this, [this, table]{ toggleMaintenanceDialog(table); });
}

void DormAdminRoomsPage::toggleMaintenanceDialog(QTableWidget* table) {
    if (!table) return;
    int r = table->currentRow();
    if (r < 0) {
        QMessageBox::information(this, "Select Room", "Please select a room first.");
        return;
    }
    QString roomNo = table->item(r, 0)->text();

    Dormitory* d = uni.findDormitory(dormId);
    if (!d) return;
    Room* room = d->findRoom(roomNo.toStdString());
    if (!room) return;

    if (!room->isUnderMaintenance()) {
        if (!room->isEmpty()) {
            QMessageBox::warning(this, "Cannot enter maintenance",
                "Reassign residents first before putting this room in maintenance.");
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

void DormAdminRoomsPage::assignDialog() {
    // Pick any unassigned student (from the whole university)
    QStringList items;
    for (const auto& s : uni.getStudents())
        if (!s.isAccommodated())
            items << QString::fromStdString(s.getId() + " – " + s.getFullName());
    if (items.isEmpty()) {
        QMessageBox::warning(this, "Unavailable", "No unassigned students available.");
        return;
    }
    bool ok = false;
    QString sel = QInputDialog::getItem(this, "Assign Student",
        "Select student:", items, 0, false, &ok);
    if (!ok) return;
    QString sid = sel.section(" – ", 0, 0);

    // Pick room in this dormitory only
    Dormitory* d = uni.findDormitory(dormId);
    if (!d) return;
    QStringList rooms;
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
        uni.assignStudentToRoom(sid.toStdString(),
                                dormId,
                                room.toStdString());
        uni.logActivity("🛏", ("Assigned " + QString::fromStdString(uni.studentName(sid.toStdString()))
                       + " to Room " + room).toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void DormAdminRoomsPage::reassignDialog() {
    // Pick only students in this dormitory
    QStringList items;
    for (const auto& s : uni.getStudents())
        if (s.isAccommodated() && s.getDormitoryId() == dormId)
            items << QString::fromStdString(
                s.getId() + " – " + s.getFullName() + " (Room " + s.getRoomNumber() + ")");
    if (items.isEmpty()) {
        QMessageBox::warning(this, "Unavailable", "No students in this dormitory to reassign.");
        return;
    }
    bool ok = false;
    QString sel = QInputDialog::getItem(this, "Reassign Student",
        "Select student:", items, 0, false, &ok);
    if (!ok) return;
    QString sid = sel.section(" – ", 0, 0);

    // Pick new room in this dormitory
    Dormitory* d = uni.findDormitory(dormId);
    if (!d) return;
    QStringList rooms;
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
        uni.reassignStudent(sid.toStdString(), dormId, room.toStdString());
        uni.logActivity("🔄", ("Reassigned " +
            QString::fromStdString(uni.studentName(sid.toStdString()))
            + " → Room " + room).toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void DormAdminRoomsPage::unassignDialog() {
    // Pick only students in this dormitory
    QStringList items;
    for (const auto& s : uni.getStudents())
        if (s.isAccommodated() && s.getDormitoryId() == dormId)
            items << QString::fromStdString(
                s.getId() + " – " + s.getFullName() + " (Room " + s.getRoomNumber() + ")");
    if (items.isEmpty()) {
        QMessageBox::warning(this, "Unavailable", "No students in this dormitory to remove.");
        return;
    }
    bool ok = false;
    QString sel = QInputDialog::getItem(this, "Remove from Room",
        "Select student:", items, 0, false, &ok);
    if (!ok) return;
    QString sid = sel.section(" – ", 0, 0);

    QString name = QString::fromStdString(uni.studentName(sid.toStdString()));
    if (QMessageBox::question(this, "Remove from Room",
            QString("Remove %1 from their room?\nThey will become unassigned.").arg(name))
        != QMessageBox::Yes) return;

    try {
        uni.removeStudentFromRoom(sid.toStdString());
        uni.logActivity("🚪", ("Removed " + name + " from room").toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  DormAdmin Students Page (read-only, filtered to dorm)
// ═══════════════════════════════════════════════════════════════════════════════

DormAdminStudentsPage::DormAdminStudentsPage(University& u, const std::string& did,
                                             QWidget* parent)
    : QWidget(parent), uni(u), dormId(did)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void DormAdminStudentsPage::applyFilters(QTableWidget* table, QLineEdit* idEdit,
                                         QLineEdit* nameEdit, QComboBox* yearBox) {
    QString sId = idEdit->text().trimmed().toLower();
    QString sName = nameEdit->text().trimmed().toLower();
    QString sYear = yearBox->currentIndex() == 0 ? "" : yearBox->currentText().replace("Year ", "");

    fStudentId = idEdit->text().toStdString();
    fName = nameEdit->text().toStdString();
    fYearIdx = yearBox->currentIndex();

    int visible = 0;
    for (int r = 0; r < table->rowCount(); ++r) {
        bool show = true;
        if (!sId.isEmpty() && table->item(r, 0))
            if (!table->item(r, 0)->text().toLower().contains(sId)) show = false;
        if (show && !sName.isEmpty() && table->item(r, 1))
            if (!table->item(r, 1)->text().toLower().contains(sName)) show = false;
        if (show && !sYear.isEmpty() && table->item(r, 2))
            if (!table->item(r, 2)->text().contains(sYear)) show = false;

        table->setRowHidden(r, !show);
        if (show) ++visible;
    }

    if (auto* countLbl = table->property("countLabel").value<QLabel*>())
        countLbl->setText(QString("Showing %1 of %2 students").arg(visible).arg(table->rowCount()));
}

void DormAdminStudentsPage::refresh() {
    clearDormLayout(root);

    Dormitory* d = uni.findDormitory(dormId);
    if (!d) { root->addWidget(new QLabel("Dormitory not found.")); return; }
    QString dormName = QString::fromStdString(d->getName());

    // Collect students in this dormitory
    std::vector<const Student*> dormStudents;
    for (const auto& s : uni.getStudents())
        if (s.getDormitoryId() == dormId)
            dormStudents.push_back(&s);

    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("👥  Students – " + dormName,
        "Students currently assigned to your dormitory. (Read-only)"));
    head->addStretch();
    root->addLayout(head);

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("👥", QString::number((int)dormStudents.size()),
                             "Total Residents", "In " + dormName, "#EEF1FF"));
    sRow->addWidget(statCard("🛏", QString::number(d->totalCapacity()),
                             "Total Capacity",  "Beds available", "#DCFCE7"));
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
    idEdit->setText(QString::fromStdString(fStudentId));

    auto* nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("Search by Name...");
    nameEdit->setMinimumWidth(140);
    nameEdit->setText(QString::fromStdString(fName));

    auto* yearCombo = new QComboBox;
    yearCombo->addItems({"All Years", "Year 1", "Year 2", "Year 3", "Year 4", "Year 5"});
    yearCombo->setMinimumWidth(110);
    if (fYearIdx < yearCombo->count()) yearCombo->setCurrentIndex(fYearIdx);

    filterRow->addWidget(idEdit);
    filterRow->addWidget(nameEdit);
    filterRow->addWidget(yearCombo);
    filterLay->addLayout(filterRow);
    root->addWidget(filterCard);

    auto* countLbl = new QLabel;
    countLbl->setStyleSheet("font-size:12px; color:#6B7280; padding:0 2px;");
    root->addWidget(countLbl);

    // Table
    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);
    auto* table = makeDormTable({"ID", "Full Name", "Year", "Room"});
    table->setProperty("countLabel", QVariant::fromValue(countLbl));

    table->setRowCount((int)dormStudents.size());
    table->setMinimumHeight(300);

    for (int i = 0; i < (int)dormStudents.size(); ++i) {
        const auto* s = dormStudents[i];
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(s->getId())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(s->getFullName())));
        table->setItem(i, 2, new QTableWidgetItem(QString("Year %1").arg(s->getAcademicYear())));
        table->setItem(i, 3, new QTableWidgetItem(
            s->getRoomNumber().empty() ? "—" : QString::fromStdString(s->getRoomNumber())));
    }

    tv->addWidget(table);
    root->addWidget(tableCard);

    auto doFilter = [this, table, idEdit, nameEdit, yearCombo]{
        applyFilters(table, idEdit, nameEdit, yearCombo);
    };
    connect(idEdit,   &QLineEdit::textChanged, this, doFilter);
    connect(nameEdit, &QLineEdit::textChanged, this, doFilter);
    connect(yearCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, doFilter);
    connect(clearBtn, &QPushButton::clicked, this, [=]{
        idEdit->clear(); nameEdit->clear(); yearCombo->setCurrentIndex(0);
    });
    doFilter();

    root->addStretch();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  DormAdmin Restaurant Page (read-only weekly menu)
// ═══════════════════════════════════════════════════════════════════════════════

DormAdminRestaurantPage::DormAdminRestaurantPage(University& u, const std::string& did,
                                                 QWidget* parent)
    : QWidget(parent), uni(u), dormId(did)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void DormAdminRestaurantPage::refresh() {
    clearDormLayout(root);

    Dormitory* d = uni.findDormitory(dormId);
    QString dormName = d ? QString::fromStdString(d->getName()) : "Dormitory";

    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("🍽  Weekly Menu – " + dormName,
        "View the centralized weekly menu for your dormitory residents."));
    head->addStretch();
    root->addLayout(head);

    // Today's highlight
    int dayOfWeek = std::max(0, QDate::currentDate().dayOfWeek() - 1);
    const auto& dailyMenu = uni.getWeeklyMenu().days[dayOfWeek];

    auto* todayCard = makeCard();
    auto* todayLay = new QVBoxLayout(todayCard);
    todayLay->setContentsMargins(24, 20, 24, 20);
    todayLay->setSpacing(10);

    auto* todayTitle = new QLabel("📅  Today's Menu");
    todayTitle->setStyleSheet("font-size:16px; font-weight:700; color:#111827;");
    todayLay->addWidget(todayTitle);
    todayLay->addWidget(menuRow("🌅", "Breakfast", QString::fromStdString(dailyMenu.breakfast)));
    todayLay->addWidget(menuRow("☀️",  "Lunch",     QString::fromStdString(dailyMenu.lunch)));
    todayLay->addWidget(menuRow("🌙", "Dinner",    QString::fromStdString(dailyMenu.dinner)));
    root->addWidget(todayCard);

    // Full weekly table
    auto* weekLabel = new QLabel("Full Weekly Schedule");
    weekLabel->setObjectName("SectionTitle");
    root->addWidget(weekLabel);

    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);

    auto* table = makeDormTable({"Day", "Breakfast", "Lunch", "Dinner"});
    table->setRowCount(7);
    table->setMinimumHeight(340);

    const char* days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    for (int i = 0; i < 7; ++i) {
        const auto& dm = uni.getWeeklyMenu().days[i];
        auto* dayItem = new QTableWidgetItem(days[i]);
        dayItem->setTextAlignment(Qt::AlignCenter);
        dayItem->setFont(QFont("Segoe UI", 11, QFont::DemiBold));
        table->setItem(i, 0, dayItem);

        auto* bf = new QTableWidgetItem(QString::fromStdString(dm.breakfast));
        bf->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 1, bf);

        auto* lu = new QTableWidgetItem(QString::fromStdString(dm.lunch));
        lu->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 2, lu);

        auto* di = new QTableWidgetItem(QString::fromStdString(dm.dinner));
        di->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 3, di);
    }

    tv->addWidget(table);
    root->addWidget(tableCard);
    root->addStretch();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  DormAdmin Meal Booking Page (read-only, filtered to dorm)
// ═══════════════════════════════════════════════════════════════════════════════

DormAdminMealBookingPage::DormAdminMealBookingPage(University& u, const std::string& did,
                                                   QWidget* parent)
    : QWidget(parent), uni(u), dormId(did)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void DormAdminMealBookingPage::refresh() {
    clearDormLayout(root);

    Dormitory* d = uni.findDormitory(dormId);
    QString dormName = d ? QString::fromStdString(d->getName()) : "Dormitory";

    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("📅  Meal Bookings – " + dormName,
        "View meal bookings for your dormitory residents. (Read-only)"));
    head->addStretch();
    root->addLayout(head);

    // Filter bookings to this dorm's students
    std::vector<const MealBooking*> dormBookings;
    for (const auto& bk : uni.getBookings()) {
        if (studentInDorm(uni, bk.studentId, dormId))
            dormBookings.push_back(&bk);
    }

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("📅", QString::number((int)dormBookings.size()),
                             "Total Bookings", "For " + dormName, "#EEF1FF"));
    sRow->addStretch();
    root->addLayout(sRow);

    auto* tableCard = makeCard();
    auto* tv = new QVBoxLayout(tableCard);
    tv->setContentsMargins(0, 0, 0, 0);

    auto* table = makeDormTable({"Student", "Date", "Meal", "Status"});
    table->setRowCount((int)dormBookings.size());
    table->setMinimumHeight(300);

    for (int i = 0; i < (int)dormBookings.size(); ++i) {
        const auto* bk = dormBookings[i];
        table->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(uni.studentName(bk->studentId))));
        table->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(bk->date)));
        table->setItem(i, 2, new QTableWidgetItem(
            QString::fromStdString(mealTypeToString(bk->meal))));
        table->setCellWidget(i, 3, pill("✅ Booked", "Available"));
    }

    tv->addWidget(table);
    root->addWidget(tableCard);

    if (dormBookings.empty()) {
        auto* empty = new QLabel("No meal bookings from your dormitory residents yet.");
        empty->setStyleSheet("font-size:13px; color:#9CA3AF; padding:10px 0;");
        root->addWidget(empty);
    }

    root->addStretch();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  DormAdmin Activity Page (read-only, Sports or Cultural)
// ═══════════════════════════════════════════════════════════════════════════════

DormAdminActivityPage::DormAdminActivityPage(University& u, const std::string& did,
                                             const std::string& cat, QWidget* parent)
    : QWidget(parent), uni(u), dormId(did), category(cat)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void DormAdminActivityPage::refresh() {
    clearDormLayout(root);

    Dormitory* d = uni.findDormitory(dormId);
    QString dormName = d ? QString::fromStdString(d->getName()) : "Dormitory";
    QString catQ = QString::fromStdString(category);
    QString emoji = (category == "Sports") ? "⚽" : "🎭";

    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader(emoji + "  " + catQ + " Activities – " + dormName,
        "View " + catQ.toLower() + " activities and enrollment status. (Read-only)"));
    head->addStretch();
    root->addLayout(head);

    bool hasAny = false;
    for (const auto& a : uni.getActivities()) {
        if (a->category() != category) continue;
        hasAny = true;

        auto* card = makeCard();
        auto* cardLay = new QHBoxLayout(card);
        cardLay->setContentsMargins(24, 18, 24, 18);
        cardLay->setSpacing(16);

        auto* icon = new QLabel(emoji);
        icon->setFixedSize(48, 48);
        icon->setAlignment(Qt::AlignCenter);
        QString iconBg = (category == "Sports") ? "#DCFCE7" : "#EDE9FE";
        icon->setStyleSheet("background: " + iconBg + "; border-radius: 14px; font-size: 22px;");

        auto* textCol = new QVBoxLayout;
        textCol->setSpacing(4);
        auto* nameL = new QLabel(QString::fromStdString(a->getName()));
        nameL->setStyleSheet("font-size: 16px; font-weight: 700; color: #111827; background: transparent;");
        auto* schedL = new QLabel("📅  " + QString::fromStdString(a->getSchedule()));
        schedL->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent;");
        auto* detailL = new QLabel(QString::fromStdString(a->describe()));
        detailL->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent;");
        detailL->setWordWrap(true);
        textCol->addWidget(nameL);
        textCol->addWidget(schedL);
        textCol->addWidget(detailL);

        // Count enrollments from this dorm's students
        int dormEnrolled = 0, totalEnrolled = 0;
        for (const auto& e : a->getEnrollments()) {
            if (e.status == "Approved") {
                ++totalEnrolled;
                if (studentInDorm(uni, e.studentId, dormId))
                    ++dormEnrolled;
            }
        }

        auto* statsCol = new QVBoxLayout;
        statsCol->setAlignment(Qt::AlignCenter);
        statsCol->setSpacing(4);
        auto* totalLbl = new QLabel(QString("👥 %1 total").arg(totalEnrolled));
        totalLbl->setStyleSheet("font-size: 13px; font-weight: 600; color: #4C6FFF;"
            " background: #EEF1FF; border-radius: 10px; padding: 6px 14px;");
        totalLbl->setAlignment(Qt::AlignCenter);
        auto* dormLbl = new QLabel(QString("🏢 %1 from %2").arg(dormEnrolled).arg(dormName));
        dormLbl->setStyleSheet("font-size: 11px; color: #6B7280; background: transparent;");
        dormLbl->setAlignment(Qt::AlignCenter);
        statsCol->addWidget(totalLbl);
        statsCol->addWidget(dormLbl);

        cardLay->addWidget(icon);
        cardLay->addLayout(textCol, 1);
        cardLay->addLayout(statsCol);

        root->addWidget(card);
    }

    if (!hasAny) {
        auto* empty = new QLabel("No " + catQ.toLower() + " activities available at this time.");
        empty->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 10px 0;");
        root->addWidget(empty);
    }

    root->addStretch();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  DormAdmin Health Page (read-only, filtered to dorm)
// ═══════════════════════════════════════════════════════════════════════════════

DormAdminHealthPage::DormAdminHealthPage(University& u, const std::string& did,
                                         QWidget* parent)
    : QWidget(parent), uni(u), dormId(did)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void DormAdminHealthPage::refresh() {
    clearDormLayout(root);

    Dormitory* d = uni.findDormitory(dormId);
    QString dormName = d ? QString::fromStdString(d->getName()) : "Dormitory";

    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("🏥  Health Clinic – " + dormName,
        "View health appointments for your dormitory residents. (Read-only)"));
    head->addStretch();
    root->addLayout(head);

    // Filter appointments to this dorm's students
    std::vector<const Appointment*> dormAppts;
    for (const auto& apt : uni.getClinic().getAppointments()) {
        if (studentInDorm(uni, apt.studentId, dormId))
            dormAppts.push_back(&apt);
    }

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    int scheduled = 0, completed = 0;
    for (const auto* apt : dormAppts) {
        if (apt->status == "Scheduled") ++scheduled;
        else if (apt->status == "Completed") ++completed;
    }
    sRow->addWidget(statCard("🏥", QString::number((int)dormAppts.size()),
                             "Total Appointments", "For " + dormName, "#EEF1FF"));
    sRow->addWidget(statCard("📋", QString::number(scheduled),
                             "Scheduled", "Upcoming", "#DBEAFE"));
    sRow->addWidget(statCard("✅", QString::number(completed),
                             "Completed", "Past visits", "#DCFCE7"));
    sRow->addStretch();
    root->addLayout(sRow);

    // Appointment cards
    bool hasAny = false;
    for (const auto* apt : dormAppts) {
        hasAny = true;

        auto* card = makeCard();
        auto* cardLay = new QHBoxLayout(card);
        cardLay->setContentsMargins(24, 18, 24, 18);
        cardLay->setSpacing(16);

        auto* icon = new QLabel("🩺");
        icon->setFixedSize(48, 48);
        icon->setAlignment(Qt::AlignCenter);
        icon->setStyleSheet("background: #DBEAFE; border-radius: 14px; font-size: 22px;");

        auto* textCol = new QVBoxLayout;
        textCol->setSpacing(4);
        auto* nameL = new QLabel(QString::fromStdString(uni.studentName(apt->studentId)));
        nameL->setStyleSheet("font-size: 15px; font-weight: 600; color: #111827; background: transparent;");
        auto* reasonL = new QLabel(QString::fromStdString(apt->reason));
        reasonL->setStyleSheet("font-size: 13px; color: #374151; background: transparent;");
        auto* dateL = new QLabel("📅 " + QString::fromStdString(apt->date) +
                                 "  🕐 " + QString::fromStdString(apt->time));
        dateL->setStyleSheet("font-size: 12px; color: #6B7280; background: transparent;");
        textCol->addWidget(nameL);
        textCol->addWidget(reasonL);
        textCol->addWidget(dateL);

        QString statusBg, statusFg;
        if (apt->status == "Scheduled")      { statusBg = "#DBEAFE"; statusFg = "#2563EB"; }
        else if (apt->status == "Completed") { statusBg = "#DCFCE7"; statusFg = "#16A34A"; }
        else                                 { statusBg = "#FEE2E2"; statusFg = "#EF4444"; }

        auto* statusL = new QLabel(QString::fromStdString(apt->status));
        statusL->setStyleSheet("font-size: 13px; font-weight: 600; color: " + statusFg +
            "; background: " + statusBg + "; border-radius: 10px; padding: 6px 16px;");
        statusL->setAlignment(Qt::AlignCenter);

        cardLay->addWidget(icon);
        cardLay->addLayout(textCol, 1);
        cardLay->addWidget(statusL, 0, Qt::AlignCenter);

        root->addWidget(card);
    }

    if (!hasAny) {
        auto* empty = new QLabel("No health appointments from your dormitory residents.");
        empty->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 10px 0;");
        root->addWidget(empty);
    }

    root->addStretch();
}

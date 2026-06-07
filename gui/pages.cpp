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

namespace {

// Remove and delete every item currently in a layout (used to rebuild on refresh).
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
    return t;
}

// "S001 - Ahmed Benali" -> picker entries; returns selected ID ("" if cancelled).
QString pickStudentId(QWidget* p, University& uni) {
    QStringList items;
    for (const auto& s : uni.getStudents())
        items << QString::fromStdString(s.getId() + " - " + s.getFullName());
    if (items.isEmpty()) { QMessageBox::warning(p, "Unavailable", "No students yet."); return {}; }
    bool ok = false;
    QString sel = QInputDialog::getItem(p, "Select Student", "Student:", items, 0, false, &ok);
    if (!ok) return {};
    return sel.section(" - ", 0, 0);
}

} // namespace

// ===================== Dashboard =====================
DashboardPage::DashboardPage(University& u, QWidget* parent) : QWidget(parent), uni(u) {
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(16);
    refresh();
}

void DashboardPage::refresh() {
    clearLayout(root);
    root->addWidget(pageHeader("Good morning, Admin", "Here's what's happening at ENSIA today."));

    auto* row = new QHBoxLayout;
    row->addWidget(statCard("\xF0\x9F\x91\xA5", QString::number(uni.totalStudents()),
                            "Total Students", "Active residents", "#EEF1FF"));
    row->addWidget(statCard("\xF0\x9F\x8F\xA2", QString::number((int)uni.getDormitories().size()),
                            "Dormitories", "On campus", "#DCFCE7"));
    row->addWidget(statCard("\xF0\x9F\x9B\x8F", QString::number(uni.availableRooms()),
                            "Available Rooms", "Ready to assign", "#FEF3C7"));
    int meals = 0;
    for (const auto& d : uni.getDormitories()) meals += d.getRestaurant().getMealsServed();
    row->addWidget(statCard("\xF0\x9F\x8D\xBD", QString::number(meals),
                            "Meals Served", "Today", "#FCE7F3"));
    root->addLayout(row);

    auto* card = makeCard();
    auto* cv = new QVBoxLayout(card);
    cv->setContentsMargins(18, 18, 18, 18);
    auto* h = new QLabel("Dormitory Occupancy");
    h->setStyleSheet("font-weight:600; font-size:16px;");
    cv->addWidget(h);
    for (const auto& d : uni.getDormitories()) {
        int cap = d.totalCapacity(), occ = d.totalOccupancy();
        int pct = cap ? occ * 100 / cap : 0;
        cv->addWidget(new QLabel(QString::fromStdString(d.getName()) +
            QString("   %1/%2 beds  (%3%)").arg(occ).arg(cap).arg(pct)));
        auto* bar = new QProgressBar;
        bar->setValue(pct); bar->setTextVisible(false); bar->setFixedHeight(8);
        bar->setStyleSheet("QProgressBar{background:#F1F2F4;border-radius:4px;}"
                           "QProgressBar::chunk{background:#4C6FFF;border-radius:4px;}");
        cv->addWidget(bar);
    }
    root->addWidget(card);
    root->addStretch();
}

// ===================== Dormitories =====================
DormitoriesPage::DormitoriesPage(University& u, QWidget* parent) : QWidget(parent), uni(u) {
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24); root->setSpacing(16);
    refresh();
}

void DormitoriesPage::refresh() {
    clearLayout(root);
    root->addWidget(pageHeader("Dormitories", "Manage dormitories, capacities, and restaurants."));
    auto* grid = new QGridLayout;
    int col = 0, r = 0;
    for (const auto& d : uni.getDormitories()) {
        auto* card = makeCard();
        auto* v = new QVBoxLayout(card); v->setContentsMargins(18, 18, 18, 18);
        auto* t = new QLabel(QString::fromStdString(d.getName()));
        t->setStyleSheet("font-weight:600; font-size:16px;");
        v->addWidget(t);
        v->addWidget(new QLabel(QString("Rooms: %1").arg(d.getRooms().size())));
        v->addWidget(new QLabel(QString("Beds: %1 / %2 occupied").arg(d.totalOccupancy()).arg(d.totalCapacity())));
        v->addWidget(new QLabel(QString("Available rooms: %1").arg(d.availableRoomCount())));
        v->addWidget(new QLabel("\xF0\x9F\x8D\xBD " + QString::fromStdString(d.getRestaurant().getName())));
        grid->addWidget(card, r, col);
        if (++col == 2) { col = 0; ++r; }
    }
    root->addLayout(grid);
    root->addStretch();
}

// ===================== Rooms =====================
RoomsPage::RoomsPage(University& u, QWidget* parent) : QWidget(parent), uni(u) {
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24); root->setSpacing(16);
    refresh();
}

void RoomsPage::refresh() {
    clearLayout(root);
    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Room Management", "View, assign, and manage all dormitory rooms."));
    head->addStretch();
    auto* addBtn = new QPushButton("+ Assign Student"); addBtn->setObjectName("Primary");
    head->addWidget(addBtn);
    root->addLayout(head);

    auto* row = new QHBoxLayout;
    row->addWidget(statCard("\xF0\x9F\x9B\x8F", QString::number(uni.totalRooms()),     "Total Rooms", "", "#EEF1FF"));
    row->addWidget(statCard("\xE2\x9C\x85", QString::number(uni.availableRooms()),     "Available Rooms", "", "#DCFCE7"));
    row->addWidget(statCard("\xF0\x9F\x91\xA5", QString::number(uni.occupiedRooms()),  "Occupied Rooms", "", "#FEF3C7"));
    root->addLayout(row);

    auto* table = makeTable({"Room", "Dormitory", "Type", "Occupancy", "Status", "Residents"});
    int total = 0; for (const auto& d : uni.getDormitories()) total += (int)d.getRooms().size();
    table->setRowCount(total);
    int i = 0;
    for (const auto& d : uni.getDormitories()) {
        for (const auto& rm : d.getRooms()) {
            table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(rm.getNumber())));
            table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(d.getName())));
            table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(rm.getType())));
            table->setItem(i, 3, new QTableWidgetItem(QString("%1/%2").arg(rm.getOccupancy()).arg(rm.getCapacity())));
            table->setCellWidget(i, 4, pill(QString::fromStdString(rm.status()), QString::fromStdString(rm.status())));
            QStringList names;
            for (const auto& occ : rm.getOccupants()) names << QString::fromStdString(uni.studentName(occ));
            table->setItem(i, 5, new QTableWidgetItem(names.isEmpty() ? "No residents" : names.join(", ")));
            ++i;
        }
    }
    root->addWidget(table);
    connect(addBtn, &QPushButton::clicked, this, [this]{ assignDialog(); });
}

void RoomsPage::assignDialog() {
    QString sid = pickStudentId(this, uni);
    if (sid.isEmpty()) return;
    QStringList dorms;
    for (const auto& d : uni.getDormitories()) dorms << QString::fromStdString(d.getId());
    bool ok = false;
    QString dorm = QInputDialog::getItem(this, "Assign Student", "Dormitory:", dorms, 0, false, &ok);
    if (!ok) return;
    QStringList rooms;
    if (auto* d = uni.findDormitory(dorm.toStdString()))
        for (const auto& r : d->getRooms())
            if (!r.isFull() && !r.isUnderMaintenance()) rooms << QString::fromStdString(r.getNumber());
    if (rooms.isEmpty()) { QMessageBox::warning(this, "Unavailable", "No available rooms in this dormitory."); return; }
    QString room = QInputDialog::getItem(this, "Assign Student", "Room:", rooms, 0, false, &ok);
    if (!ok) return;
    try {
        uni.assignStudentToRoom(sid.toStdString(), dorm.toStdString(), room.toStdString());
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

// ===================== Students =====================
StudentsPage::StudentsPage(University& u, QWidget* parent) : QWidget(parent), uni(u) {
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24); root->setSpacing(16);
    refresh();
}

void StudentsPage::refresh() {
    clearLayout(root);
    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Students", "Manage student records and accommodation."));
    head->addStretch();
    auto* rmBtn  = new QPushButton("Remove Selected");
    auto* addBtn = new QPushButton("+ Add Student"); addBtn->setObjectName("Primary");
    head->addWidget(rmBtn); head->addWidget(addBtn);
    root->addLayout(head);

    auto* table = makeTable({"ID", "Full Name", "Year", "Accommodation"});
    auto& studs = uni.getStudents();
    table->setRowCount((int)studs.size());
    for (int i = 0; i < (int)studs.size(); ++i) {
        const auto& s = studs[i];
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(s.getId())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(s.getFullName())));
        table->setItem(i, 2, new QTableWidgetItem(QString("Year %1").arg(s.getAcademicYear())));
        table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(s.accommodationStatus())));
    }
    root->addWidget(table);

    connect(addBtn, &QPushButton::clicked, this, [this]{ addDialog(); });
    connect(rmBtn, &QPushButton::clicked, this, [this, table]{
        int r = table->currentRow();
        if (r < 0) { QMessageBox::information(this, "Remove", "Select a student row first."); return; }
        QString id = table->item(r, 0)->text();
        try { uni.removeStudent(id.toStdString()); refresh(); }
        catch (const std::exception& e) { QMessageBox::warning(this, "Error", e.what()); }
    });
}

void StudentsPage::addDialog() {
    bool ok = false;
    QString id = QInputDialog::getText(this, "Add Student", "Student ID:", QLineEdit::Normal, "", &ok);
    if (!ok || id.trimmed().isEmpty()) return;
    QString name = QInputDialog::getText(this, "Add Student", "Full name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    int year = QInputDialog::getInt(this, "Add Student", "Academic year (1-5):", 1, 1, 5, 1, &ok);
    if (!ok) return;
    try {
        uni.addStudent(Student(id.toStdString(), name.toStdString(), year));
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

// ===================== Restaurant =====================
RestaurantPage::RestaurantPage(University& u, QWidget* parent) : QWidget(parent), uni(u) {
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24); root->setSpacing(16);
    refresh();
}

void RestaurantPage::refresh() {
    clearLayout(root);
    root->addWidget(pageHeader("Restaurant & Menus", "Manage daily menus for each dormitory restaurant."));
    for (auto& d : uni.getDormitories()) {
        auto* card = makeCard();
        auto* v = new QVBoxLayout(card); v->setContentsMargins(18, 18, 18, 18);
        auto* t = new QLabel(QString::fromStdString(d.getRestaurant().getName()));
        t->setStyleSheet("font-weight:600; font-size:16px;");
        v->addWidget(t);
        const Menu& m = d.getRestaurant().getMenu();
        auto* b  = new QLineEdit(QString::fromStdString(m.breakfast));
        auto* l  = new QLineEdit(QString::fromStdString(m.lunch));
        auto* dn = new QLineEdit(QString::fromStdString(m.dinner));
        v->addWidget(new QLabel("Breakfast")); v->addWidget(b);
        v->addWidget(new QLabel("Lunch"));     v->addWidget(l);
        v->addWidget(new QLabel("Dinner"));    v->addWidget(dn);
        auto* hb = new QHBoxLayout;
        auto* save  = new QPushButton("Save Menu"); save->setObjectName("Primary");
        auto* serve = new QPushButton("Serve Meal (+1)");
        auto* served = new QLabel(QString("Meals served today: %1").arg(d.getRestaurant().getMealsServed()));
        hb->addWidget(save); hb->addWidget(serve); hb->addStretch(); hb->addWidget(served);
        v->addLayout(hb);
        std::string dormId = d.getId();
        connect(save, &QPushButton::clicked, this, [this, dormId, b, l, dn]{
            if (auto* dd = uni.findDormitory(dormId))
                dd->getRestaurant().setMenu(Menu(b->text().toStdString(), l->text().toStdString(), dn->text().toStdString()));
            QMessageBox::information(this, "Saved", "Menu updated.");
        });
        connect(serve, &QPushButton::clicked, this, [this, dormId]{
            if (auto* dd = uni.findDormitory(dormId)) dd->getRestaurant().serveMeal();
            refresh();
        });
        root->addWidget(card);
    }
    root->addStretch();
}

// ===================== Meal Booking =====================
MealBookingPage::MealBookingPage(University& u, QWidget* parent) : QWidget(parent), uni(u) {
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24); root->setSpacing(16);
    refresh();
}

void MealBookingPage::refresh() {
    clearLayout(root);
    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Meal Booking", "Residents can book breakfast, lunch, or dinner in advance."));
    head->addStretch();
    auto* btn = new QPushButton("+ Book Meal"); btn->setObjectName("Primary");
    head->addWidget(btn);
    root->addLayout(head);

    auto* table = makeTable({"Student", "Date", "Meal", "Status"});
    auto& bk = uni.getBookings();
    table->setRowCount((int)bk.size());
    for (int i = 0; i < (int)bk.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(uni.studentName(bk[i].studentId))));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(bk[i].date)));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(mealTypeToString(bk[i].meal))));
        table->setItem(i, 3, new QTableWidgetItem(bk[i].confirmed ? "Confirmed" : "Pending"));
    }
    root->addWidget(table);
    connect(btn, &QPushButton::clicked, this, [this]{ bookDialog(); });
}

void MealBookingPage::bookDialog() {
    QString sid = pickStudentId(this, uni);
    if (sid.isEmpty()) return;
    bool ok = false;
    QString date = QInputDialog::getText(this, "Book Meal", "Date (YYYY-MM-DD):", QLineEdit::Normal, "2026-06-08", &ok);
    if (!ok || date.trimmed().isEmpty()) return;
    QString meal = QInputDialog::getItem(this, "Book Meal", "Meal:", {"Breakfast", "Lunch", "Dinner"}, 1, false, &ok);
    if (!ok) return;
    try {
        uni.bookMeal(MealBooking(sid.toStdString(), date.toStdString(), mealTypeFromString(meal.toStdString())));
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

// ===================== Health Clinic =====================
HealthPage::HealthPage(University& u, QWidget* parent) : QWidget(parent), uni(u) {
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24); root->setSpacing(16);
    refresh();
}

void HealthPage::refresh() {
    clearLayout(root);
    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("Health Clinic", "Manage appointments and student health records."));
    head->addStretch();
    auto* btn = new QPushButton("+ Schedule Appointment"); btn->setObjectName("Primary");
    head->addWidget(btn);
    root->addLayout(head);

    auto* table = makeTable({"Student", "Date", "Time", "Reason", "Status"});
    auto& ap = uni.getClinic().getAppointments();
    table->setRowCount((int)ap.size());
    for (int i = 0; i < (int)ap.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(uni.studentName(ap[i].studentId))));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(ap[i].date)));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(ap[i].time)));
        table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(ap[i].reason)));
        table->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(ap[i].status)));
    }
    root->addWidget(table);
    connect(btn, &QPushButton::clicked, this, [this]{ scheduleDialog(); });
}

void HealthPage::scheduleDialog() {
    QString sid = pickStudentId(this, uni);
    if (sid.isEmpty()) return;
    bool ok = false;
    QString date = QInputDialog::getText(this, "Schedule", "Date (YYYY-MM-DD):", QLineEdit::Normal, "2026-06-08", &ok);
    if (!ok || date.trimmed().isEmpty()) return;
    QString time = QInputDialog::getText(this, "Schedule", "Time (HH:MM):", QLineEdit::Normal, "10:00", &ok);
    if (!ok) return;
    QString reason = QInputDialog::getText(this, "Schedule", "Reason:", QLineEdit::Normal, "Check-up", &ok);
    if (!ok) return;
    try {
        uni.getClinic().schedule(Appointment(sid.toStdString(), date.toStdString(), time.toStdString(), reason.toStdString()));
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

// ===================== Activities (Sports / Cultural) =====================
ActivityPage::ActivityPage(University& u, QString cat, QWidget* parent)
    : QWidget(parent), uni(u), category(std::move(cat)) {
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24); root->setSpacing(16);
    refresh();
}

void ActivityPage::refresh() {
    clearLayout(root);
    const bool sports = (category == "Sports");
    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader(sports ? "Sports Activities" : "Cultural Activities",
        sports ? "Register and manage sports programs."
               : "Manage cultural and recreational activities."));
    head->addStretch();
    auto* enrollBtn = new QPushButton("Enroll Student");
    auto* addBtn = new QPushButton("+ Add Activity"); addBtn->setObjectName("Primary");
    head->addWidget(enrollBtn); head->addWidget(addBtn);
    root->addLayout(head);

    for (const auto& a : uni.getActivities()) {
        if (QString::fromStdString(a->category()) != category) continue;
        auto* card = makeCard();
        auto* v = new QVBoxLayout(card); v->setContentsMargins(18, 18, 18, 18);
        auto* t = new QLabel(QString::fromStdString(a->getName()));
        t->setStyleSheet("font-weight:600; font-size:15px;");
        v->addWidget(t);
        v->addWidget(new QLabel(QString::fromStdString(a->describe())));
        v->addWidget(new QLabel(QString("Participants: %1").arg(a->getParticipants().size())));
        root->addWidget(card);
    }
    root->addStretch();
    connect(addBtn, &QPushButton::clicked, this, [this]{ addDialog(); });
    connect(enrollBtn, &QPushButton::clicked, this, [this]{ enrollDialog(); });
}

void ActivityPage::addDialog() {
    const bool sports = (category == "Sports");
    bool ok = false;
    QString name = QInputDialog::getText(this, "Add Activity", "Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    QString sched = QInputDialog::getText(this, "Add Activity", "Schedule:", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    QString extra = QInputDialog::getText(this, "Add Activity", sports ? "Coach:" : "Venue:", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    if (sports) uni.addActivity(std::make_unique<SportsActivity>(name.toStdString(), sched.toStdString(), extra.toStdString()));
    else        uni.addActivity(std::make_unique<CulturalActivity>(name.toStdString(), sched.toStdString(), extra.toStdString()));
    refresh();
}

void ActivityPage::enrollDialog() {
    QStringList acts;
    for (const auto& a : uni.getActivities())
        if (QString::fromStdString(a->category()) == category) acts << QString::fromStdString(a->getName());
    if (acts.isEmpty()) { QMessageBox::warning(this, "Unavailable", "No activities yet."); return; }
    bool ok = false;
    QString an = QInputDialog::getItem(this, "Enroll", "Activity:", acts, 0, false, &ok);
    if (!ok) return;
    QString sid = pickStudentId(this, uni);
    if (sid.isEmpty()) return;
    for (const auto& a : uni.getActivities()) {
        if (QString::fromStdString(a->getName()) == an && QString::fromStdString(a->category()) == category) {
            try { a->enroll(sid.toStdString()); }
            catch (const std::exception& e) { QMessageBox::warning(this, "Error", e.what()); }
            break;
        }
    }
    refresh();
}
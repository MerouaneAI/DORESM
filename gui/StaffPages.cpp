#include "gui/StaffPages.h"
#include "gui/widgets.h"
#include "model/University.h"
#include "model/WeeklyMenu.h"
#include "model/MealBooking.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>
#include <QDate>
#include <algorithm>

// ─── Helpers ─────────────────────────────────────────────────────────────────

namespace {

void clearStaffLayout(QLayout* layout) {
    if (!layout) return;
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        if (item->layout()) clearStaffLayout(item->layout());
        delete item;
    }
}

QTableWidget* makeStaffTable(const QStringList& cols) {
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

} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
//  Staff Dashboard
// ═══════════════════════════════════════════════════════════════════════════════

StaffDashboardPage::StaffDashboardPage(University& u, QWidget* parent)
    : QWidget(parent), uni(u)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void StaffDashboardPage::refresh() {
    clearStaffLayout(root);

    root->addWidget(pageHeader("Good morning, Staff 👋",
                               "Here's your maintenance and meal service overview."));

    // ── Count stats ──────────────────────────────────────────────────
    int maintRooms = 0;
    for (const auto& d : uni.getDormitories())
        for (const auto& r : d.getRooms())
            if (r.isUnderMaintenance()) ++maintRooms;

    int dayOfWeek = std::max(0, QDate::currentDate().dayOfWeek() - 1);
    int mealsServed = uni.getWeeklyMenu().days[dayOfWeek].mealsServed;

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    int todayBookings = 0, todayUnserved = 0;
    for (const auto& b : uni.getBookings()) {
        if (b.date == today.toStdString()) {
            ++todayBookings;
            if (!b.served) ++todayUnserved;
        }
    }

    auto* statRow = new QHBoxLayout;
    statRow->setSpacing(16);
    statRow->addWidget(statCard("🔧", QString::number(maintRooms),
                                "Rooms in Maintenance", "Awaiting repair", "#FEF3C7", "#D97706"));
    statRow->addWidget(statCard("🍽", QString::number(mealsServed),
                                "Meals Served Today", "Total served", "#DCFCE7"));
    statRow->addWidget(statCard("📅", QString::number(todayBookings),
                                "Today's Bookings", QString("%1 unserved").arg(todayUnserved), "#EEF1FF"));
    statRow->addWidget(statCard("🏢", QString::number((int)uni.getDormitories().size()),
                                "Dormitories", "Total on campus", "#FCE7F3"));
    root->addLayout(statRow);

    // ── Today's Menu preview ─────────────────────────────────────────
    auto* menuCard = makeCard();
    auto* menuLay = new QVBoxLayout(menuCard);
    menuLay->setContentsMargins(24, 20, 24, 20);
    menuLay->setSpacing(10);

    auto* menuTitle = new QLabel("📅  Today's Menu");
    menuTitle->setStyleSheet("font-size:16px; font-weight:700; color:#111827;");
    menuLay->addWidget(menuTitle);

    const auto& dailyMenu = uni.getWeeklyMenu().days[dayOfWeek];
    menuLay->addWidget(menuRow("🌅", "Breakfast", QString::fromStdString(dailyMenu.breakfast)));
    menuLay->addWidget(menuRow("☀️",  "Lunch",     QString::fromStdString(dailyMenu.lunch)));
    menuLay->addWidget(menuRow("🌙", "Dinner",    QString::fromStdString(dailyMenu.dinner)));
    root->addWidget(menuCard);

    // ── Maintenance summary ──────────────────────────────────────────
    if (maintRooms > 0) {
        auto* maintTitle = new QLabel("🔧  Rooms Awaiting Repair");
        maintTitle->setStyleSheet("font-size:16px; font-weight:700; color:#111827;");
        root->addWidget(maintTitle);

        int shown = 0;
        for (const auto& d : uni.getDormitories()) {
            for (const auto& r : d.getRooms()) {
                if (!r.isUnderMaintenance()) continue;
                if (++shown > 5) break;

                auto* card = makeCard();
                auto* cardLay = new QHBoxLayout(card);
                cardLay->setContentsMargins(20, 14, 20, 14);
                cardLay->setSpacing(14);

                auto* icon = new QLabel("🔧");
                icon->setFixedSize(40, 40);
                icon->setAlignment(Qt::AlignCenter);
                icon->setStyleSheet("background:#FEF3C7; border-radius:12px; font-size:18px;");

                auto* info = new QVBoxLayout;
                info->setSpacing(2);
                auto* roomLbl = new QLabel("Room " + QString::fromStdString(r.getNumber()));
                roomLbl->setStyleSheet("font-size:14px; font-weight:600; color:#111827;");
                auto* dormLbl = new QLabel(QString::fromStdString(d.getName()));
                dormLbl->setStyleSheet("font-size:12px; color:#6B7280;");
                info->addWidget(roomLbl);
                info->addWidget(dormLbl);

                auto* statusL = new QLabel("⚠ Under Maintenance");
                statusL->setStyleSheet("font-size:12px; font-weight:600; color:#D97706;"
                    " background:#FEF3C7; border-radius:8px; padding:5px 12px;");

                cardLay->addWidget(icon);
                cardLay->addLayout(info, 1);
                cardLay->addWidget(statusL, 0, Qt::AlignCenter);
                root->addWidget(card);
            }
            if (shown > 5) break;
        }
    }

    root->addStretch();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Staff Maintenance Page
// ═══════════════════════════════════════════════════════════════════════════════

StaffMaintenancePage::StaffMaintenancePage(University& u, QWidget* parent)
    : QWidget(parent), uni(u)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void StaffMaintenancePage::refresh() {
    clearStaffLayout(root);

    // Count maintenance rooms
    int maintRooms = 0;
    for (const auto& d : uni.getDormitories())
        for (const auto& r : d.getRooms())
            if (r.isUnderMaintenance()) ++maintRooms;

    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("🔧  Room Maintenance",
        "View all rooms under maintenance and mark them as fixed."));
    head->addStretch();
    root->addLayout(head);

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("🔧", QString::number(maintRooms),
                             "Under Maintenance", "Awaiting repair", "#FEF3C7", "#D97706"));
    sRow->addWidget(statCard("🛏", QString::number(uni.totalRooms()),
                             "Total Rooms", "All dormitories", "#EEF1FF"));
    sRow->addStretch();
    root->addLayout(sRow);

    // ── Maintenance room cards ───────────────────────────────────────
    if (maintRooms == 0) {
        auto* empty = new QLabel("✅  All rooms are in good condition. No maintenance needed!");
        empty->setStyleSheet("font-size:14px; color:#16A34A; padding:20px 0;");
        root->addWidget(empty);
    } else {
        for (auto& d : uni.getDormitories()) {
            for (auto& r : d.getRooms()) {
                if (!r.isUnderMaintenance()) continue;

                auto* card = makeCard();
                auto* cardLay = new QHBoxLayout(card);
                cardLay->setContentsMargins(24, 18, 24, 18);
                cardLay->setSpacing(16);

                auto* icon = new QLabel("🔧");
                icon->setFixedSize(48, 48);
                icon->setAlignment(Qt::AlignCenter);
                icon->setStyleSheet("background:#FEF3C7; border-radius:14px; font-size:22px;");

                auto* textCol = new QVBoxLayout;
                textCol->setSpacing(4);
                auto* roomLbl = new QLabel("Room " + QString::fromStdString(r.getNumber()));
                roomLbl->setStyleSheet("font-size:16px; font-weight:700; color:#111827; background:transparent;");
                auto* dormLbl = new QLabel("🏢  " + QString::fromStdString(d.getName()));
                dormLbl->setStyleSheet("font-size:13px; color:#6B7280; background:transparent;");
                auto* typeLbl = new QLabel(QString::fromStdString(r.getType()) +
                    "  ·  Capacity: " + QString::number(r.getCapacity()));
                typeLbl->setStyleSheet("font-size:12px; color:#9CA3AF; background:transparent;");
                textCol->addWidget(roomLbl);
                textCol->addWidget(dormLbl);
                textCol->addWidget(typeLbl);

                auto* fixBtn = new QPushButton("✅ Mark as Fixed");
                fixBtn->setCursor(Qt::PointingHandCursor);
                fixBtn->setStyleSheet(
                    "QPushButton { background:#10B981; color:#FFFFFF; border:none; border-radius:10px;"
                    " padding:10px 24px; font-size:13px; font-weight:600; }"
                    "QPushButton:hover { background:#059669; }");

                QString dormId = QString::fromStdString(d.getId());
                QString roomNo = QString::fromStdString(r.getNumber());
                connect(fixBtn, &QPushButton::clicked, this, [this, dormId, roomNo]() {
                    Dormitory* dorm = uni.findDormitory(dormId.toStdString());
                    if (!dorm) return;
                    Room* room = dorm->findRoom(roomNo.toStdString());
                    if (!room) return;
                    room->setMaintenance(false);
                    uni.logActivity("✅", "Staff fixed room " + roomNo.toStdString() +
                                    " in " + dorm->getName());
                    QMessageBox::information(this, "Fixed",
                        "Room " + roomNo + " has been marked as fixed and is now available.");
                    refresh();
                });

                cardLay->addWidget(icon);
                cardLay->addLayout(textCol, 1);
                cardLay->addWidget(fixBtn, 0, Qt::AlignCenter);
                root->addWidget(card);
            }
        }
    }

    root->addStretch();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Staff Restaurant Page (serve meals)
// ═══════════════════════════════════════════════════════════════════════════════

StaffRestaurantPage::StaffRestaurantPage(University& u, QWidget* parent)
    : QWidget(parent), uni(u)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void StaffRestaurantPage::refresh() {
    clearStaffLayout(root);

    int dayOfWeek = std::max(0, QDate::currentDate().dayOfWeek() - 1);
    int mealsServed = uni.getWeeklyMenu().days[dayOfWeek].mealsServed;
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    // Count today's bookings
    int todayTotal = 0, todayUnserved = 0, todayServed = 0;
    for (const auto& b : uni.getBookings()) {
        if (b.date == today.toStdString()) {
            ++todayTotal;
            if (b.served) ++todayServed;
            else ++todayUnserved;
        }
    }

    auto* head = new QHBoxLayout;
    head->addWidget(pageHeader("🍽  Serve Meals",
        "View today's menu and serve meals to students who booked."));
    head->addStretch();
    root->addLayout(head);

    auto* sRow = new QHBoxLayout; sRow->setSpacing(16);
    sRow->addWidget(statCard("🍽", QString::number(mealsServed),
                             "Meals Served Today", "Total count", "#DCFCE7"));
    sRow->addWidget(statCard("📋", QString::number(todayUnserved),
                             "Pending Bookings", "To be served", "#FEF3C7", "#D97706"));
    sRow->addWidget(statCard("✅", QString::number(todayServed),
                             "Served Bookings", "Completed", "#EEF1FF"));
    sRow->addStretch();
    root->addLayout(sRow);

    // ── Today's Menu ─────────────────────────────────────────────────
    auto* menuCard = makeCard();
    auto* menuLay = new QVBoxLayout(menuCard);
    menuLay->setContentsMargins(24, 20, 24, 20);
    menuLay->setSpacing(10);

    auto* menuTitle = new QLabel("📅  Today's Menu");
    menuTitle->setStyleSheet("font-size:16px; font-weight:700; color:#111827;");
    menuLay->addWidget(menuTitle);

    const auto& dailyMenu = uni.getWeeklyMenu().days[dayOfWeek];
    menuLay->addWidget(menuRow("🌅", "Breakfast", QString::fromStdString(dailyMenu.breakfast)));
    menuLay->addWidget(menuRow("☀️",  "Lunch",     QString::fromStdString(dailyMenu.lunch)));
    menuLay->addWidget(menuRow("🌙", "Dinner",    QString::fromStdString(dailyMenu.dinner)));
    root->addWidget(menuCard);

    // ── Pending Bookings (unserved, today only) ──────────────────────
    auto* pendingTitle = new QLabel("📋  Pending Meal Bookings — Today");
    pendingTitle->setStyleSheet("font-size:17px; font-weight:700; color:#111827;");
    root->addWidget(pendingTitle);

    bool hasPending = false;
    for (size_t idx = 0; idx < uni.getBookings().size(); ++idx) {
        auto& bk = uni.getBookings()[idx];
        if (bk.date != today.toStdString() || bk.served) continue;
        hasPending = true;

        auto* card = makeCard();
        auto* cardLay = new QHBoxLayout(card);
        cardLay->setContentsMargins(24, 18, 24, 18);
        cardLay->setSpacing(16);

        QString mealStr = QString::fromStdString(mealTypeToString(bk.meal));
        QString mealEmoji = "🍽";
        QString mealBg = "#FEF3C7";
        if (mealStr == "Breakfast") { mealEmoji = "☕"; mealBg = "#FEF3C7"; }
        else if (mealStr == "Lunch") { mealEmoji = "🥘"; mealBg = "#DCFCE7"; }
        else if (mealStr == "Dinner") { mealEmoji = "🍝"; mealBg = "#DBEAFE"; }

        auto* icon = new QLabel(mealEmoji);
        icon->setFixedSize(48, 48);
        icon->setAlignment(Qt::AlignCenter);
        icon->setStyleSheet("background:" + mealBg + "; border-radius:14px; font-size:22px;");

        auto* textCol = new QVBoxLayout;
        textCol->setSpacing(4);
        auto* studentLbl = new QLabel(QString::fromStdString(uni.studentName(bk.studentId)));
        studentLbl->setStyleSheet("font-size:15px; font-weight:600; color:#111827; background:transparent;");
        auto* mealLbl = new QLabel(mealStr + "  ·  " + today);
        mealLbl->setStyleSheet("font-size:13px; color:#6B7280; background:transparent;");
        auto* idLbl = new QLabel("ID: " + QString::fromStdString(bk.studentId));
        idLbl->setStyleSheet("font-size:11px; color:#9CA3AF; background:transparent;");
        textCol->addWidget(studentLbl);
        textCol->addWidget(mealLbl);
        textCol->addWidget(idLbl);

        auto* serveBtn = new QPushButton("🍽 Serve Meal");
        serveBtn->setCursor(Qt::PointingHandCursor);
        serveBtn->setStyleSheet(
            "QPushButton { background:#4C6FFF; color:#FFFFFF; border:none; border-radius:10px;"
            " padding:10px 24px; font-size:13px; font-weight:600; }"
            "QPushButton:hover { background:#3B5FFF; }");

        size_t bookingIdx = idx;
        connect(serveBtn, &QPushButton::clicked, this, [this, bookingIdx, dayOfWeek]() {
            if (bookingIdx >= uni.getBookings().size()) return;
            auto& booking = uni.getBookings()[bookingIdx];
            if (booking.served) return; // already served

            booking.served = true;
            uni.getWeeklyMenu().days[dayOfWeek].mealsServed++;
            uni.logActivity("🍽", "Served " +
                mealTypeToString(booking.meal) + " to " +
                uni.studentName(booking.studentId));
            refresh();
        });

        cardLay->addWidget(icon);
        cardLay->addLayout(textCol, 1);
        cardLay->addWidget(serveBtn, 0, Qt::AlignCenter);
        root->addWidget(card);
    }

    if (!hasPending) {
        auto* empty = new QLabel("✅  All today's bookings have been served!");
        empty->setStyleSheet("font-size:13px; color:#16A34A; padding:10px 0;");
        root->addWidget(empty);
    }

    // ── Already Served (today) ───────────────────────────────────────
    bool hasServed = false;
    for (const auto& bk : uni.getBookings()) {
        if (bk.date != today.toStdString() || !bk.served) continue;
        if (!hasServed) {
            auto* servedTitle = new QLabel("✅  Served Today");
            servedTitle->setStyleSheet("font-size:17px; font-weight:700; color:#111827;");
            root->addWidget(servedTitle);
            hasServed = true;
        }

        auto* card = makeCard();
        auto* cardLay = new QHBoxLayout(card);
        cardLay->setContentsMargins(24, 14, 24, 14);
        cardLay->setSpacing(14);

        QString mealStr = QString::fromStdString(mealTypeToString(bk.meal));
        auto* icon = new QLabel("✅");
        icon->setFixedSize(40, 40);
        icon->setAlignment(Qt::AlignCenter);
        icon->setStyleSheet("background:#DCFCE7; border-radius:12px; font-size:18px;");

        auto* info = new QVBoxLayout;
        info->setSpacing(2);
        auto* nm = new QLabel(QString::fromStdString(uni.studentName(bk.studentId)));
        nm->setStyleSheet("font-size:14px; font-weight:600; color:#111827; background:transparent;");
        auto* ml = new QLabel(mealStr);
        ml->setStyleSheet("font-size:12px; color:#6B7280; background:transparent;");
        info->addWidget(nm);
        info->addWidget(ml);

        auto* badge = new QLabel("Served");
        badge->setStyleSheet("font-size:12px; font-weight:600; color:#16A34A;"
            " background:#DCFCE7; border-radius:8px; padding:5px 14px;");

        cardLay->addWidget(icon);
        cardLay->addLayout(info, 1);
        cardLay->addWidget(badge, 0, Qt::AlignCenter);
        root->addWidget(card);
    }

    root->addStretch();
}

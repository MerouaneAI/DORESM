#include "gui/StudentPages.h"
#include "gui/widgets.h"
#include "model/University.h"
#include "model/Activity.h"
#include "model/WeeklyMenu.h"
#include "model/MealBooking.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QScrollArea>
#include <QComboBox>

// ─── Helpers ──────────────────────────────────────────────────────────────────

static void clearStudentLayout(QLayout* layout) {
    if (!layout) return;
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        if (item->layout()) clearStudentLayout(item->layout());
        delete item;
    }
}

static QFrame* infoCard(const QString& emoji, const QString& label, const QString& value, const QString& accentBg) {
    auto* card = new QFrame;
    card->setStyleSheet(
        "QFrame { background: #FFFFFF; border-radius: 14px; border: 1px solid #E5E7EB; }");
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto* lay = new QHBoxLayout(card);
    lay->setContentsMargins(20, 18, 20, 18);
    lay->setSpacing(14);

    auto* icon = new QLabel(emoji);
    icon->setFixedSize(48, 48);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background:" + accentBg + "; border-radius:14px; font-size:22px;");

    auto* textCol = new QVBoxLayout;
    textCol->setSpacing(2);

    auto* labelW = new QLabel(label);
    labelW->setStyleSheet("font-size: 12px; font-weight: 600; color: #6B7280; background: transparent;");

    auto* valueW = new QLabel(value);
    valueW->setStyleSheet("font-size: 16px; font-weight: 700; color: #111827; background: transparent;");

    textCol->addWidget(labelW);
    textCol->addWidget(valueW);

    lay->addWidget(icon);
    lay->addLayout(textCol);
    lay->addStretch();

    return card;
}

// ─── Student Dashboard ────────────────────────────────────────────────────────

StudentDashboardPage::StudentDashboardPage(University& u, const QString& sid, QWidget* parent)
    : QWidget(parent), uni(u), studentId(sid)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void StudentDashboardPage::refresh() {
    clearStudentLayout(root);

    Student* s = uni.findStudent(studentId.toStdString());
    QString studentName = s ? QString::fromStdString(s->getFullName()) : "Student";

    // ── Header ───────────────────────────────────────────────────────
    auto* header = new QLabel("Welcome, " + studentName + " 👋");
    header->setStyleSheet("font-size: 26px; font-weight: 700; color: #111827;");
    root->addWidget(header);

    auto* sub = new QLabel("Here's your student portal overview.");
    sub->setStyleSheet("font-size: 13px; color: #6B7280; margin-top: -8px;");
    root->addWidget(sub);

    // ── Info Cards Row ───────────────────────────────────────────────
    auto* infoRow = new QHBoxLayout;
    infoRow->setSpacing(16);

    infoRow->addWidget(infoCard("🏛", "University", QString::fromStdString(uni.getName()), "#EEF1FF"));

    QString dormName = "Not Assigned";
    if (s && s->isAccommodated()) {
        if (auto* d = uni.findDormitory(s->getDormitoryId()))
            dormName = QString::fromStdString(d->getName());
    }
    infoRow->addWidget(infoCard("🏢", "Dormitory", dormName, "#FEF3C7"));

    QString roomNum = (s && s->isAccommodated()) ? QString::fromStdString(s->getRoomNumber()) : "Not Assigned";
    infoRow->addWidget(infoCard("🛏", "Room Number", roomNum, "#DCFCE7"));

    QString year = s ? ("Year " + QString::number(s->getAcademicYear())) : "—";
    infoRow->addWidget(infoCard("📚", "Academic Year", year, "#DBEAFE"));

    root->addLayout(infoRow);

    // ── Weekly Meal Plan ─────────────────────────────────────────────
    auto* mealTitle = new QLabel("📅  Weekly Meal Plan");
    mealTitle->setStyleSheet("font-size: 17px; font-weight: 700; color: #111827;");
    root->addWidget(mealTitle);

    auto* menuCard = makeCard();
    auto* menuLay = new QVBoxLayout(menuCard);
    menuLay->setContentsMargins(0, 0, 0, 0);

    auto* table = new QTableWidget;
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"Day", "Breakfast", "Lunch", "Dinner"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(44);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setShowGrid(false);
    table->setFrameShape(QFrame::NoFrame);
    table->setRowCount(7);
    table->setMinimumHeight(340);

    const char* days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    for (int i = 0; i < 7; ++i) {
        const auto& d = uni.getWeeklyMenu().days[i];
        auto* dayItem = new QTableWidgetItem(days[i]);
        dayItem->setTextAlignment(Qt::AlignCenter);
        dayItem->setFont(QFont("Segoe UI", 11, QFont::DemiBold));
        table->setItem(i, 0, dayItem);

        auto* bf = new QTableWidgetItem(QString::fromStdString(d.breakfast));
        bf->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 1, bf);

        auto* lu = new QTableWidgetItem(QString::fromStdString(d.lunch));
        lu->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 2, lu);

        auto* di = new QTableWidgetItem(QString::fromStdString(d.dinner));
        di->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 3, di);
    }

    menuLay->addWidget(table);
    root->addWidget(menuCard);

    root->addStretch();
}

// ─── Student Events Page ──────────────────────────────────────────────────────

StudentEventsPage::StudentEventsPage(University& u, const QString& sid, QWidget* parent)
    : QWidget(parent), uni(u), studentId(sid)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

static QWidget* buildEventCard(
    University& uni, const QString& studentId,
    const std::unique_ptr<Activity>& a,
    const QString& emoji, const QString& iconBg,
    QWidget* parent, std::function<void()> refreshFn)
{
    auto* card = makeCard();
    auto* cardLay = new QHBoxLayout(card);
    cardLay->setContentsMargins(24, 18, 24, 18);
    cardLay->setSpacing(16);

    auto* icon = new QLabel(emoji);
    icon->setFixedSize(48, 48);
    icon->setAlignment(Qt::AlignCenter);
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

    // Check enrollment status
    std::string status = a->enrollmentStatus(studentId.toStdString());

    auto* rightCol = new QVBoxLayout;
    rightCol->setAlignment(Qt::AlignCenter);
    rightCol->setSpacing(8);

    if (status == "Approved") {
        auto* statusL = new QLabel("✅ Approved");
        statusL->setStyleSheet("font-size: 13px; font-weight: 600; color: #16A34A;"
            " background: #DCFCE7; border-radius: 10px; padding: 6px 16px;");
        statusL->setAlignment(Qt::AlignCenter);
        rightCol->addWidget(statusL);

        auto* withdrawBtn = new QPushButton("Withdraw");
        withdrawBtn->setCursor(Qt::PointingHandCursor);
        withdrawBtn->setStyleSheet(
            "QPushButton { background: #FEE2E2; color: #EF4444; border: none; border-radius: 8px;"
            " padding: 8px 20px; font-size: 12px; font-weight: 600; }"
            "QPushButton:hover { background: #FECACA; }");
        rightCol->addWidget(withdrawBtn);

        QString actName = QString::fromStdString(a->getName());
        QObject::connect(withdrawBtn, &QPushButton::clicked, parent, [&uni, actName, studentId, refreshFn]() {
            for (auto& act : uni.getActivities()) {
                if (QString::fromStdString(act->getName()) == actName) {
                    try { act->withdraw(studentId.toStdString()); } catch (...) {}
                    break;
                }
            }
            refreshFn();
        });

    } else if (status == "Pending") {
        auto* statusL = new QLabel("⏳ Pending");
        statusL->setStyleSheet("font-size: 13px; font-weight: 600; color: #D97706;"
            " background: #FEF3C7; border-radius: 10px; padding: 6px 16px;");
        statusL->setAlignment(Qt::AlignCenter);
        rightCol->addWidget(statusL);

        auto* cancelBtn = new QPushButton("Cancel");
        cancelBtn->setCursor(Qt::PointingHandCursor);
        cancelBtn->setStyleSheet(
            "QPushButton { background: #F3F4F6; color: #6B7280; border: none; border-radius: 8px;"
            " padding: 8px 20px; font-size: 12px; font-weight: 600; }"
            "QPushButton:hover { background: #E5E7EB; }");
        rightCol->addWidget(cancelBtn);

        QString actName = QString::fromStdString(a->getName());
        QObject::connect(cancelBtn, &QPushButton::clicked, parent, [&uni, actName, studentId, refreshFn]() {
            for (auto& act : uni.getActivities()) {
                if (QString::fromStdString(act->getName()) == actName) {
                    try { act->withdraw(studentId.toStdString()); } catch (...) {}
                    break;
                }
            }
            refreshFn();
        });

    } else if (status == "Refused") {
        auto* statusL = new QLabel("❌ Refused");
        statusL->setStyleSheet("font-size: 13px; font-weight: 600; color: #EF4444;"
            " background: #FEE2E2; border-radius: 10px; padding: 6px 16px;");
        statusL->setAlignment(Qt::AlignCenter);
        rightCol->addWidget(statusL);

    } else {
        // Not applied yet
        auto* applyBtn = new QPushButton("📝 Apply");
        applyBtn->setCursor(Qt::PointingHandCursor);
        applyBtn->setStyleSheet(
            "QPushButton { background: #4C6FFF; color: #FFFFFF; border: none; border-radius: 10px;"
            " padding: 10px 24px; font-size: 13px; font-weight: 600; }"
            "QPushButton:hover { background: #3B5FFF; }");
        rightCol->addWidget(applyBtn);

        QString actName = QString::fromStdString(a->getName());
        QObject::connect(applyBtn, &QPushButton::clicked, parent, [&uni, actName, studentId, refreshFn]() {
            for (auto& act : uni.getActivities()) {
                if (QString::fromStdString(act->getName()) == actName) {
                    try {
                        act->apply(studentId.toStdString());
                        uni.logActivity("📝", uni.studentName(studentId.toStdString()) +
                            " applied to " + actName.toStdString());
                    } catch (const std::exception& ex) {
                        QMessageBox::warning(nullptr, "Error", ex.what());
                    }
                    break;
                }
            }
            refreshFn();
        });
    }

    cardLay->addWidget(icon);
    cardLay->addLayout(textCol, 1);
    cardLay->addLayout(rightCol);

    return card;
}

void StudentEventsPage::refresh() {
    clearStudentLayout(root);

    // ── Header ───────────────────────────────────────────────────────
    auto* head = new QHBoxLayout;
    auto* headerW = new QWidget;
    auto* headerLay = new QVBoxLayout(headerW);
    headerLay->setContentsMargins(0,0,0,0);
    headerLay->setSpacing(2);
    auto* title = new QLabel("🎯  Events & Activities");
    title->setStyleSheet("font-size: 24px; font-weight: 700; color: #111827;");
    auto* subtitle = new QLabel("Apply for sports and cultural events. Your application will be reviewed by an admin.");
    subtitle->setStyleSheet("font-size: 13px; color: #6B7280;");
    headerLay->addWidget(title);
    headerLay->addWidget(subtitle);
    head->addWidget(headerW);
    head->addStretch();
    root->addLayout(head);

    auto refreshFn = [this]() { refresh(); };

    // ── Sports Events ────────────────────────────────────────────────
    auto* sportsLabel = new QLabel("⚽  Sports Activities");
    sportsLabel->setStyleSheet("font-size: 17px; font-weight: 700; color: #111827;");
    root->addWidget(sportsLabel);

    bool hasSports = false;
    for (const auto& a : uni.getActivities()) {
        if (a->category() != "Sports") continue;
        hasSports = true;
        root->addWidget(buildEventCard(uni, studentId, a, "⚽", "#DCFCE7", this, refreshFn));
    }
    if (!hasSports) {
        auto* empty = new QLabel("No sports events available at this time.");
        empty->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 10px 0;");
        root->addWidget(empty);
    }

    root->addSpacing(10);

    // ── Cultural Events ──────────────────────────────────────────────
    auto* culturalLabel = new QLabel("🎭  Cultural Activities");
    culturalLabel->setStyleSheet("font-size: 17px; font-weight: 700; color: #111827;");
    root->addWidget(culturalLabel);

    bool hasCultural = false;
    for (const auto& a : uni.getActivities()) {
        if (a->category() != "Cultural") continue;
        hasCultural = true;
        root->addWidget(buildEventCard(uni, studentId, a, "🎭", "#EDE9FE", this, refreshFn));
    }
    if (!hasCultural) {
        auto* empty = new QLabel("No cultural events available at this time.");
        empty->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 10px 0;");
        root->addWidget(empty);
    }

    root->addStretch();
}

// ─── Student Health Page ──────────────────────────────────────────────────────

StudentHealthPage::StudentHealthPage(University& u, const QString& sid, QWidget* parent)
    : QWidget(parent), uni(u), studentId(sid)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void StudentHealthPage::refresh() {
    clearStudentLayout(root);

    // ── Header ───────────────────────────────────────────────────────
    auto* head = new QHBoxLayout;
    auto* headerW = new QWidget;
    auto* headerLay = new QVBoxLayout(headerW);
    headerLay->setContentsMargins(0,0,0,0);
    headerLay->setSpacing(2);
    auto* title = new QLabel("🏥  Health Clinic");
    title->setStyleSheet("font-size: 24px; font-weight: 700; color: #111827;");
    auto* subtitle = new QLabel("Book an appointment with a doctor.");
    subtitle->setStyleSheet("font-size: 13px; color: #6B7280;");
    headerLay->addWidget(title);
    headerLay->addWidget(subtitle);
    head->addWidget(headerW);
    head->addStretch();

    auto* bookBtn = new QPushButton("+ Book Appointment");
    bookBtn->setCursor(Qt::PointingHandCursor);
    bookBtn->setStyleSheet(
        "QPushButton { background: #4C6FFF; color: #FFFFFF; border: none; border-radius: 10px;"
        " padding: 10px 24px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #3B5FFF; }");
    head->addWidget(bookBtn);
    root->addLayout(head);

    // ── My Appointments ──────────────────────────────────────────────
    auto* myTitle = new QLabel("📋  My Appointments");
    myTitle->setStyleSheet("font-size: 17px; font-weight: 700; color: #111827;");
    root->addWidget(myTitle);

    bool hasAppointments = false;
    for (const auto& apt : uni.getClinic().getAppointments()) {
        if (apt.studentId != studentId.toStdString()) continue;
        hasAppointments = true;

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
        auto* reasonL = new QLabel(QString::fromStdString(apt.reason));
        reasonL->setStyleSheet("font-size: 15px; font-weight: 600; color: #111827; background: transparent;");
        auto* dateL = new QLabel("📅 " + QString::fromStdString(apt.date) + "  🕐 " + QString::fromStdString(apt.time));
        dateL->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent;");
        textCol->addWidget(reasonL);
        textCol->addWidget(dateL);

        QString statusBg, statusFg;
        if (apt.status == "Scheduled") { statusBg = "#DBEAFE"; statusFg = "#2563EB"; }
        else if (apt.status == "Completed") { statusBg = "#DCFCE7"; statusFg = "#16A34A"; }
        else { statusBg = "#FEE2E2"; statusFg = "#EF4444"; }

        auto* statusL = new QLabel(QString::fromStdString(apt.status));
        statusL->setStyleSheet("font-size: 13px; font-weight: 600; color: " + statusFg +
            "; background: " + statusBg + "; border-radius: 10px; padding: 6px 16px;");
        statusL->setAlignment(Qt::AlignCenter);

        cardLay->addWidget(icon);
        cardLay->addLayout(textCol, 1);
        cardLay->addWidget(statusL, 0, Qt::AlignCenter);

        root->addWidget(card);
    }

    if (!hasAppointments) {
        auto* empty = new QLabel("You have no appointments yet. Book one above!");
        empty->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 10px 0;");
        root->addWidget(empty);
    }

    root->addStretch();

    connect(bookBtn, &QPushButton::clicked, this, &StudentHealthPage::bookAppointmentDialog);
}

void StudentHealthPage::bookAppointmentDialog() {
    bool ok = false;
    QString date = QInputDialog::getText(this, "Book Appointment", "Date (YYYY-MM-DD):",
                                         QLineEdit::Normal, "2026-06-10", &ok);
    if (!ok || date.trimmed().isEmpty()) return;

    QString time = QInputDialog::getText(this, "Book Appointment", "Time (HH:MM):",
                                         QLineEdit::Normal, "09:00", &ok);
    if (!ok || time.trimmed().isEmpty()) return;

    QString reason = QInputDialog::getText(this, "Book Appointment", "Reason:",
                                           QLineEdit::Normal, "", &ok);
    if (!ok || reason.trimmed().isEmpty()) return;

    uni.getClinic().schedule(
        Appointment(studentId.toStdString(), date.toStdString(),
                    time.toStdString(), reason.toStdString()));

    uni.logActivity("🏥", "Student " + uni.studentName(studentId.toStdString()) +
                    " booked appointment for " + date.toStdString());

    QMessageBox::information(this, "Booked", "Your appointment has been booked successfully!");
    refresh();
}

// ─── Student Meal Booking Page ────────────────────────────────────────────────

StudentMealBookingPage::StudentMealBookingPage(University& u, const QString& sid, QWidget* parent)
    : QWidget(parent), uni(u), studentId(sid)
{
    root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);
    refresh();
}

void StudentMealBookingPage::refresh() {
    clearStudentLayout(root);

    // ── Header ───────────────────────────────────────────────────────
    auto* head = new QHBoxLayout;
    auto* headerW = new QWidget;
    auto* headerLay = new QVBoxLayout(headerW);
    headerLay->setContentsMargins(0,0,0,0);
    headerLay->setSpacing(2);
    auto* title = new QLabel("🍽  Meal Booking");
    title->setStyleSheet("font-size: 24px; font-weight: 700; color: #111827;");
    auto* subtitle = new QLabel("Book your meals in advance for any day.");
    subtitle->setStyleSheet("font-size: 13px; color: #6B7280;");
    headerLay->addWidget(title);
    headerLay->addWidget(subtitle);
    head->addWidget(headerW);
    head->addStretch();

    auto* bookBtn = new QPushButton("+ Book Meal");
    bookBtn->setCursor(Qt::PointingHandCursor);
    bookBtn->setStyleSheet(
        "QPushButton { background: #4C6FFF; color: #FFFFFF; border: none; border-radius: 10px;"
        " padding: 10px 24px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #3B5FFF; }");
    head->addWidget(bookBtn);
    root->addLayout(head);

    // ── My Bookings ──────────────────────────────────────────────────
    auto* myTitle = new QLabel("📋  My Meal Bookings");
    myTitle->setStyleSheet("font-size: 17px; font-weight: 700; color: #111827;");
    root->addWidget(myTitle);

    bool hasBookings = false;
    for (const auto& bk : uni.getBookings()) {
        if (bk.studentId != studentId.toStdString()) continue;
        hasBookings = true;

        auto* card = makeCard();
        auto* cardLay = new QHBoxLayout(card);
        cardLay->setContentsMargins(24, 18, 24, 18);
        cardLay->setSpacing(16);

        QString mealEmoji = "🍽";
        QString mealBg = "#FEF3C7";
        std::string mealStr = mealTypeToString(bk.meal);
        if (mealStr == "Breakfast") { mealEmoji = "☕"; mealBg = "#FEF3C7"; }
        else if (mealStr == "Lunch") { mealEmoji = "🥘"; mealBg = "#DCFCE7"; }
        else if (mealStr == "Dinner") { mealEmoji = "🍝"; mealBg = "#DBEAFE"; }

        auto* icon = new QLabel(mealEmoji);
        icon->setFixedSize(48, 48);
        icon->setAlignment(Qt::AlignCenter);
        icon->setStyleSheet("background: " + mealBg + "; border-radius: 14px; font-size: 22px;");

        auto* textCol = new QVBoxLayout;
        textCol->setSpacing(4);
        auto* mealL = new QLabel(QString::fromStdString(mealStr));
        mealL->setStyleSheet("font-size: 15px; font-weight: 600; color: #111827; background: transparent;");
        auto* dateL = new QLabel("📅 " + QString::fromStdString(bk.date));
        dateL->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent;");
        textCol->addWidget(mealL);
        textCol->addWidget(dateL);

        auto* statusL = new QLabel("✅ Booked");
        statusL->setStyleSheet("font-size: 13px; font-weight: 600; color: #16A34A;"
            " background: #DCFCE7; border-radius: 10px; padding: 6px 16px;");
        statusL->setAlignment(Qt::AlignCenter);

        cardLay->addWidget(icon);
        cardLay->addLayout(textCol, 1);
        cardLay->addWidget(statusL, 0, Qt::AlignCenter);

        root->addWidget(card);
    }

    if (!hasBookings) {
        auto* empty = new QLabel("You have no meal bookings yet. Book one above!");
        empty->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 10px 0;");
        root->addWidget(empty);
    }

    root->addStretch();

    connect(bookBtn, &QPushButton::clicked, this, &StudentMealBookingPage::bookMealDialog);
}

void StudentMealBookingPage::bookMealDialog() {
    bool ok = false;
    QString date = QInputDialog::getText(this, "Book Meal", "Date (YYYY-MM-DD):",
                                         QLineEdit::Normal, "2026-06-10", &ok);
    if (!ok || date.trimmed().isEmpty()) return;

    QString meal = QInputDialog::getItem(this, "Book Meal", "Meal:",
                                         {"Breakfast", "Lunch", "Dinner"}, 1, false, &ok);
    if (!ok) return;

    try {
        uni.bookMeal(MealBooking(studentId.toStdString(), date.toStdString(),
                                  mealTypeFromString(meal.toStdString())));
        uni.logActivity("🍽", uni.studentName(studentId.toStdString()) +
                        " booked " + meal.toStdString() + " on " + date.toStdString());
        QMessageBox::information(this, "Booked", "Your meal has been booked successfully!");
        refresh();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

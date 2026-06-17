#include "gui/LoginWindow.h"
#include "model/University.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>

LoginWindow::LoginWindow(University& u, QWidget* parent)
    : QWidget(parent), uni(u)
{
    setWindowTitle("DORESM – Login");
    setMinimumSize(900, 600);

    // Background
    setStyleSheet("LoginWindow { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1C1F3B, stop:0.5 #2A2F5B, stop:1 #1C1F3B); }");

    auto* mainLay = new QHBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);

    // ── Left: Branding Panel ─────────────────────────────────────────
    auto* leftPanel = new QWidget;
    leftPanel->setFixedWidth(380);
    leftPanel->setStyleSheet("background: transparent;");
    auto* leftLay = new QVBoxLayout(leftPanel);
    leftLay->setContentsMargins(50, 0, 50, 0);
    leftLay->setAlignment(Qt::AlignCenter);

    auto* brandIcon = new QLabel("🏛");
    brandIcon->setAlignment(Qt::AlignCenter);
    brandIcon->setStyleSheet("font-size: 64px; background: transparent;");
    leftLay->addWidget(brandIcon);
    leftLay->addSpacing(16);

    auto* brandTitle = new QLabel("DORESM");
    brandTitle->setAlignment(Qt::AlignCenter);
    brandTitle->setStyleSheet("font-size: 36px; font-weight: 800; color: #FFFFFF; background: transparent; letter-spacing: 3px;");
    leftLay->addWidget(brandTitle);

    auto* brandSub = new QLabel("Dormitory & Restaurant\nManagement System");
    brandSub->setAlignment(Qt::AlignCenter);
    brandSub->setStyleSheet("font-size: 13px; color: #9CA3AF; background: transparent; line-height: 1.5;");
    leftLay->addWidget(brandSub);

    leftLay->addSpacing(30);

    auto* divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color: rgba(255,255,255,0.12); background: rgba(255,255,255,0.12); max-width: 100px;");
    divider->setFixedHeight(1);
    leftLay->addWidget(divider, 0, Qt::AlignCenter);

    leftLay->addSpacing(20);

    auto* tagline = new QLabel("University 2025–26");
    tagline->setAlignment(Qt::AlignCenter);
    tagline->setStyleSheet("font-size: 12px; color: #6B7280; background: transparent; letter-spacing: 2px;");
    leftLay->addWidget(tagline);

    mainLay->addWidget(leftPanel);

    // ── Right: Login Card ────────────────────────────────────────────
    auto* rightPanel = new QWidget;
    rightPanel->setStyleSheet("background: transparent;");
    auto* rightLay = new QVBoxLayout(rightPanel);
    rightLay->setAlignment(Qt::AlignCenter);
    rightLay->setContentsMargins(60, 0, 60, 0);

    auto* card = new QFrame;
    card->setStyleSheet(
        "QFrame { background: #FFFFFF; border-radius: 20px; }"
    );
    card->setFixedSize(380, 440);

    // Shadow on card
    auto* shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 60));
    card->setGraphicsEffect(shadow);

    auto* cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(40, 40, 40, 36);
    cardLay->setSpacing(0);

    auto* welcomeLabel = new QLabel("Welcome back");
    welcomeLabel->setStyleSheet("font-size: 24px; font-weight: 700; color: #111827; background: transparent;");
    cardLay->addWidget(welcomeLabel);

    auto* subLabel = new QLabel("Sign in to your account");
    subLabel->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent; margin-top: 4px;");
    cardLay->addWidget(subLabel);

    cardLay->addSpacing(28);

    // Username
    auto* userLabel = new QLabel("Username");
    userLabel->setStyleSheet("font-size: 12px; font-weight: 600; color: #374151; background: transparent;");
    cardLay->addWidget(userLabel);
    cardLay->addSpacing(6);

    usernameEdit = new QLineEdit;
    usernameEdit->setPlaceholderText("Enter your username");
    usernameEdit->setStyleSheet(
        "QLineEdit { background: #F9FAFB; border: 1px solid #E5E7EB; border-radius: 10px;"
        " padding: 12px 14px; font-size: 14px; color: #111827; }"
        "QLineEdit:focus { border: 2px solid #4C6FFF; background: #FFFFFF; }");
    cardLay->addWidget(usernameEdit);

    cardLay->addSpacing(16);

    // Password
    auto* passLabel = new QLabel("Password");
    passLabel->setStyleSheet("font-size: 12px; font-weight: 600; color: #374151; background: transparent;");
    cardLay->addWidget(passLabel);
    cardLay->addSpacing(6);

    passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("Enter your password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(
        "QLineEdit { background: #F9FAFB; border: 1px solid #E5E7EB; border-radius: 10px;"
        " padding: 12px 14px; font-size: 14px; color: #111827; }"
        "QLineEdit:focus { border: 2px solid #4C6FFF; background: #FFFFFF; }");
    cardLay->addWidget(passwordEdit);

    cardLay->addSpacing(10);

    // Error label
    errorLabel = new QLabel("");
    errorLabel->setStyleSheet("font-size: 12px; color: #EF4444; background: transparent; font-weight: 500;");
    errorLabel->setAlignment(Qt::AlignCenter);
    cardLay->addWidget(errorLabel);

    cardLay->addSpacing(10);

    // Login button
    auto* loginBtn = new QPushButton("Sign In");
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setStyleSheet(
        "QPushButton { background: #4C6FFF; color: #FFFFFF; border: none; border-radius: 10px;"
        " padding: 13px 0; font-size: 15px; font-weight: 600; }"
        "QPushButton:hover { background: #3B5FFF; }"
        "QPushButton:pressed { background: #2A4FEE; }");
    cardLay->addWidget(loginBtn);

    cardLay->addStretch();



    rightLay->addWidget(card, 0, Qt::AlignCenter);
    mainLay->addWidget(rightPanel, 1);

    // Connections
    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::attemptLogin);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::attemptLogin);
    connect(usernameEdit, &QLineEdit::returnPressed, this, [this]{ passwordEdit->setFocus(); });
}

void LoginWindow::attemptLogin() {
    QString user = usernameEdit->text().trimmed();
    QString pass = passwordEdit->text().trimmed();

    if (user.isEmpty() || pass.isEmpty()) {
        errorLabel->setText("Please enter both username and password.");
        return;
    }

    // Admin login
    if (user == "admin" && pass == "admin") {
        emit adminLoggedIn();
        return;
    }

    // Staff login: username = "Staff X" (case-insensitive), password = "staffX"
    for (const auto& d : uni.getDormitories()) {
        QString staffUser = "Staff " + QString::fromStdString(d.getId());
        QString staffPass = "staff" + QString::fromStdString(d.getId());
        if (user.compare(staffUser, Qt::CaseInsensitive) == 0 && pass == staffPass) {
            emit staffLoggedIn(QString::fromStdString(d.getId()));
            return;
        }
    }

    // Dorm admin login: username = dormitory name, password = dorm ID
    for (const auto& d : uni.getDormitories()) {
        if (QString::fromStdString(d.getName()).compare(user, Qt::CaseInsensitive) == 0 &&
            QString::fromStdString(d.getId()) == pass) {
            emit dormAdminLoggedIn(QString::fromStdString(d.getId()));
            return;
        }
    }

    // Student login: username = full name, password = student ID
    for (const auto& s : uni.getStudents()) {
        if (QString::fromStdString(s.getFullName()).compare(user, Qt::CaseInsensitive) == 0 &&
            QString::fromStdString(s.getId()) == pass) {
            emit studentLoggedIn(QString::fromStdString(s.getId()));
            return;
        }
    }

    errorLabel->setText("Invalid credentials. Please try again.");
}

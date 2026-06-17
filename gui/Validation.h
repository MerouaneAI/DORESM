#pragma once
#include <QString>
#include <QDate>
#include <QRegularExpression>

// ─── Input Validation Utilities ──────────────────────────────────────────────
// Centralised validation for all user inputs in DORESM.
// Each function returns an empty string on success, or an error message on failure.

namespace Validate {

// ── Student ID ───────────────────────────────────────────────────────────────
// Format: digits only (e.g. 001, 42, 1234)
inline QString studentId(const QString& id) {
    QString v = id.trimmed();
    if (v.isEmpty())
        return "Student ID cannot be empty.";
    static QRegularExpression re("^\\d+$");
    if (!re.match(v).hasMatch())
        return "Invalid Student ID format.\n\n"
               "Student ID must contain only digits (numbers).\n"
               "Examples: 001, 042, 1234";
    return {};
}

// ── Student Full Name ────────────────────────────────────────────────────────
// Must have at least first and last name, letters and spaces only
inline QString studentName(const QString& name) {
    QString v = name.trimmed();
    if (v.isEmpty())
        return "Student name cannot be empty.";
    if (v.length() < 3)
        return "Name is too short. Please enter the student's full name.";
    // Must contain only letters, spaces, hyphens, and apostrophes
    static QRegularExpression re("^[A-Za-zÀ-ÿ][A-Za-zÀ-ÿ' -]+$");
    if (!re.match(v).hasMatch())
        return "Invalid name format.\n\n"
               "Name must contain only letters, spaces, hyphens, or apostrophes.\n"
               "Examples: Ahmed Benali, Sara Khelifi, Jean-Pierre O'Brien";
    // Must have at least 2 words (first + last name)
    if (v.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size() < 2)
        return "Please enter the student's full name (first and last name).\n\n"
               "Examples: Ahmed Benali, Sara Khelifi";
    return {};
}

// ── Dormitory ID ─────────────────────────────────────────────────────────────
// Single uppercase letter A-Z
inline QString dormitoryId(const QString& id) {
    QString v = id.trimmed();
    if (v.isEmpty())
        return "Dormitory ID cannot be empty.";
    static QRegularExpression re("^[A-Z]$");
    if (!re.match(v).hasMatch())
        return "Invalid Dormitory ID format.\n\n"
               "Must be a single uppercase letter (A-Z).\n"
               "Examples: A, B, E";
    return {};
}

// ── Dormitory Name ───────────────────────────────────────────────────────────
inline QString dormitoryName(const QString& name) {
    QString v = name.trimmed();
    if (v.isEmpty())
        return "Dormitory name cannot be empty.";
    if (v.length() < 3)
        return "Dormitory name is too short (minimum 3 characters).";
    if (v.length() > 50)
        return "Dormitory name is too long (maximum 50 characters).";
    return {};
}

// ── Room Number ──────────────────────────────────────────────────────────────
// Format: X-NNN where X is the dormitory ID letter and NNN are digits
inline QString roomNumber(const QString& num, const QString& dormId) {
    QString v = num.trimmed();
    if (v.isEmpty())
        return "Room number cannot be empty.";
    // Pattern: dormId letter, dash, then 3 digits
    QString pattern = "^" + QRegularExpression::escape(dormId) + "-\\d{3}$";
    QRegularExpression re(pattern);
    if (!re.match(v).hasMatch())
        return QString("Invalid room number format.\n\n"
               "Expected format: %1-NNN (where NNN are 3 digits)\n"
               "Examples: %1-101, %1-205, %1-310").arg(dormId);
    return {};
}

// ── Date ─────────────────────────────────────────────────────────────────────
// User-facing format: dd-mm-yyyy
// Returns the validated QDate (caller converts to internal yyyy-MM-dd)
inline QString date(const QString& dateStr, QDate* outDate = nullptr) {
    QString v = dateStr.trimmed();
    if (v.isEmpty())
        return "Date cannot be empty.";
    // Try parsing as dd-mm-yyyy
    QDate d = QDate::fromString(v, "dd-MM-yyyy");
    if (!d.isValid())
        return "Invalid date format.\n\n"
               "Expected format: dd-mm-yyyy\n"
               "Examples: 08-06-2026, 25-12-2026";
    if (outDate) *outDate = d;
    return {};
}

// ── Date that must be today or in the future ─────────────────────────────────
inline QString futureDate(const QString& dateStr, QDate* outDate = nullptr) {
    QDate d;
    QString err = date(dateStr, &d);
    if (!err.isEmpty()) return err;
    if (d < QDate::currentDate())
        return "Date cannot be in the past.\n\n"
               "Please enter today's date or a future date.";
    if (outDate) *outDate = d;
    return {};
}

// ── Time ─────────────────────────────────────────────────────────────────────
// Format: HH:MM (24-hour)
inline QString time(const QString& timeStr) {
    QString v = timeStr.trimmed();
    if (v.isEmpty())
        return "Time cannot be empty.";
    static QRegularExpression re("^([01]\\d|2[0-3]):[0-5]\\d$");
    if (!re.match(v).hasMatch())
        return "Invalid time format.\n\n"
               "Expected format: HH:MM (24-hour clock)\n"
               "Examples: 09:00, 14:30, 22:15\n"
               "Hours: 00-23, Minutes: 00-59";
    return {};
}

// ── Non-empty text with descriptive label ────────────────────────────────────
inline QString notEmpty(const QString& text, const QString& fieldName) {
    if (text.trimmed().isEmpty())
        return fieldName + " cannot be empty.\n\nPlease provide a valid " +
               fieldName.toLower() + ".";
    return {};
}

// ── Menu item text ───────────────────────────────────────────────────────────
inline QString menuItem(const QString& text, const QString& mealName) {
    if (text.trimmed().isEmpty())
        return mealName + " menu cannot be empty.\n\n"
               "Please describe the meal (e.g. \"Bread, Jam, Coffee\").";
    return {};
}

// ── Activity Name ────────────────────────────────────────────────────────────
inline QString activityName(const QString& name) {
    QString v = name.trimmed();
    if (v.isEmpty())
        return "Activity name cannot be empty.";
    if (v.length() < 2)
        return "Activity name is too short (minimum 2 characters).";
    if (v.length() > 60)
        return "Activity name is too long (maximum 60 characters).";
    return {};
}

// ── Utility: convert user date (dd-mm-yyyy) to internal (yyyy-MM-dd) ─────────
inline QString toInternal(const QString& userDate) {
    QDate d = QDate::fromString(userDate.trimmed(), "dd-MM-yyyy");
    return d.toString("yyyy-MM-dd");
}

// ── Utility: convert internal date (yyyy-MM-dd) to display (dd-mm-yyyy) ──────
inline QString toDisplay(const std::string& internalDate) {
    QDate d = QDate::fromString(QString::fromStdString(internalDate), "yyyy-MM-dd");
    if (!d.isValid()) return QString::fromStdString(internalDate);
    return d.toString("dd-MM-yyyy");
}

inline QString toDisplay(const QString& internalDate) {
    QDate d = QDate::fromString(internalDate, "yyyy-MM-dd");
    if (!d.isValid()) return internalDate;
    return d.toString("dd-MM-yyyy");
}

// ── Today's date in user format ──────────────────────────────────────────────
inline QString todayUserFormat() {
    return QDate::currentDate().toString("dd-MM-yyyy");
}

} // namespace Validate

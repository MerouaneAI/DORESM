#pragma once
#include <QString>

static const QString APP_STYLESHEET = R"(
/* ── Global ─────────────────────────────────────────── */
QWidget {
    font-family: "Segoe UI", "Inter", sans-serif;
    font-size: 13px;
    color: #111827;
    background: transparent;
}
QMainWindow, QScrollArea, QScrollArea > QWidget > QWidget {
    background: #F3F4F8;
}
QScrollBar:vertical {
    background: #F3F4F8; width: 6px; border: none;
}
QScrollBar::handle:vertical {
    background: #D1D5DB; border-radius: 3px; min-height: 30px;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

/* ── Sidebar ─────────────────────────────────────────── */
#Sidebar {
    background: #1C1F3B;
    border-right: none;
}
#Brand {
    font-size: 16px; font-weight: 700;
    color: #FFFFFF; padding: 4px 6px 0 6px;
}
#BrandSub {
    font-size: 11px; color: #9CA3AF; padding: 0 6px 8px 6px;
}
#SideMenuLabel {
    font-size: 10px; font-weight: 600; color: #6B7280;
    letter-spacing: 1px; padding: 10px 14px 4px 14px;
}
#NavBtn {
    text-align: left;
    padding: 9px 14px;
    color: #CBD5E1;
    background: transparent;
    border: none;
    border-radius: 8px;
    font-size: 13px;
    font-weight: 400;
}
#NavBtn:hover {
    background: rgba(255,255,255,0.07);
    color: #FFFFFF;
}
#NavBtn:checked {
    background: #4C6FFF;
    color: #FFFFFF;
    font-weight: 600;
}
#SideAdminBox {
    border-top: 1px solid rgba(255,255,255,0.08);
    padding: 12px 14px 4px 14px;
}
#SideAdminName { font-size: 13px; font-weight: 600; color: #F9FAFB; }
#SideAdminRole { font-size: 11px; color: #9CA3AF; }

/* ── Content area ───────────────────────────────────── */
QScrollArea#Content {
    background: #F3F4F8;
    border: none;
}

/* ── Cards ──────────────────────────────────────────── */
#Card {
    background: #FFFFFF;
    border-radius: 14px;
    border: 1px solid #E5E7EB;
}

/* ── Stat cards ──────────────────────────────────────── */
#StatValue { font-size: 28px; font-weight: 700; color: #111827; }
#StatLabel { font-size: 13px; font-weight: 600; color: #374151; }
#StatSub   { font-size: 12px; color: #6B7280; }
#StatTrend { font-size: 12px; color: #16A34A; }

/* ── Page header ─────────────────────────────────────── */
#PageTitle    { font-size: 24px; font-weight: 700; color: #111827; }
#PageSubtitle { font-size: 13px; color: #6B7280; margin-top: 2px; }

/* ── Section label ───────────────────────────────────── */
#SectionTitle { font-size: 17px; font-weight: 700; color: #111827; }

/* ── Module card ─────────────────────────────────────── */
#ModuleCard {
    background: #FFFFFF;
    border-radius: 14px;
    border: 1px solid #E5E7EB;
}
#ModuleTitle { font-size: 15px; font-weight: 600; color: #111827; }
#ModuleDesc  { font-size: 12px; color: #6B7280; }
#ModuleOpen  { font-size: 12px; color: #4C6FFF; font-weight: 500; }

/* ── Right panel ─────────────────────────────────────── */
#PanelCard {
    background: #FFFFFF;
    border-radius: 14px;
    border: 1px solid #E5E7EB;
}
#PanelTitle { font-size: 15px; font-weight: 700; color: #111827; }
#PanelSub   { font-size: 12px; color: #6B7280; }

/* ── Progress bar ────────────────────────────────────── */
QProgressBar {
    background: #F1F2F4; border-radius: 4px; border: none;
}
QProgressBar::chunk { background: #4C6FFF; border-radius: 4px; }
#OrangeBar::chunk   { background: #F59E0B; }
#GreenBar::chunk    { background: #10B981; }
#RedBar::chunk      { background: #EF4444; }

/* ── Tables ──────────────────────────────────────────── */
QTableWidget {
    background: #FFFFFF;
    border: 1px solid #E5E7EB;
    border-radius: 12px;
    gridline-color: #F3F4F8;
    outline: none;
}
QTableWidget::item { padding: 10px 12px; color: #374151; border: none; }
QTableWidget::item:selected {
    background: #EEF1FF; color: #4C6FFF;
}
QHeaderView::section {
    background: #F9FAFB;
    color: #6B7280;
    font-weight: 600;
    font-size: 11px;
    letter-spacing: 0.5px;
    text-transform: uppercase;
    padding: 10px 12px;
    border: none;
    border-bottom: 1px solid #E5E7EB;
}

/* ── Buttons ─────────────────────────────────────────── */
QPushButton {
    background: #FFFFFF;
    color: #374151;
    border: 1px solid #D1D5DB;
    border-radius: 8px;
    padding: 7px 16px;
    font-weight: 500;
}
QPushButton:hover { background: #F9FAFB; }
QPushButton#Primary {
    background: #4C6FFF;
    color: #FFFFFF;
    border: none;
}
QPushButton#Primary:hover { background: #3B5FFF; }

/* ── Inputs ──────────────────────────────────────────── */
QLineEdit {
    background: #F9FAFB;
    border: 1px solid #E5E7EB;
    border-radius: 8px;
    padding: 7px 12px;
    color: #111827;
}
QLineEdit:focus { border-color: #4C6FFF; background: #FFF; }

/* ── Activity row ────────────────────────────────────── */
#ActivityRow {
    background: #FFFFFF;
    border-bottom: 1px solid #F3F4F8;
    padding: 10px 0;
}
#ActivityText { font-size: 13px; color: #374151; }
#ActivityTime { font-size: 11px; color: #9CA3AF; }

/* ── Menu card ───────────────────────────────────────── */
#MenuCard {
    background: #FFF7ED;
    border-radius: 12px;
    border: 1px solid #FDE68A;
}
#MenuTitle { font-size: 14px; font-weight: 700; color: #92400E; }
#MenuItem  { font-size: 12px; color: #78350F; }

/* ── Event row ───────────────────────────────────────── */
#EventDate { font-size: 11px; color: #9CA3AF; }
#EventName { font-size: 13px; font-weight: 500; color: #111827; }

/* ── Dialogs (QInputDialog / QMessageBox) ────────────── */
QDialog, QInputDialog, QMessageBox {
    background: #FFFFFF;
}
QDialog QLabel, QInputDialog QLabel, QMessageBox QLabel {
    background: transparent;
    color: #111827;
    font-size: 13px;
}

/* ── Combo boxes (used by the selection dialogs) ─────── */
QComboBox {
    background: #F9FAFB;
    border: 1px solid #E5E7EB;
    border-radius: 8px;
    padding: 6px 12px;
    color: #111827;
    min-height: 18px;
}
QComboBox:focus { border-color: #4C6FFF; background: #FFFFFF; }
QComboBox::drop-down { border: none; width: 22px; }
QComboBox QAbstractItemView {
    background: #FFFFFF;
    color: #111827;
    border: 1px solid #E5E7EB;
    border-radius: 8px;
    outline: none;
    selection-background-color: #EEF1FF;
    selection-color: #4C6FFF;
}

/* ── Spin boxes (used by the numeric "year" dialog) ──── */
QSpinBox {
    background: #F9FAFB;
    border: 1px solid #E5E7EB;
    border-radius: 8px;
    padding: 6px 10px;
    color: #111827;
}
QSpinBox:focus { border-color: #4C6FFF; background: #FFFFFF; }
)";
#pragma once

inline const char* APP_STYLESHEET = R"qss(
QWidget { font-family: 'Segoe UI', 'Inter', Arial; font-size: 14px; color: #1F2937; }
#Sidebar { background: #FFFFFF; border-right: 1px solid #EEF0F4; }
#Content { background: #F6F7F9; }
#Brand { font-size: 16px; font-weight: 700; color: #1F2937; }
#BrandSub { color: #9CA3AF; font-size: 11px; }
QPushButton#NavBtn { text-align: left; padding: 10px 14px; border: none;
    border-radius: 10px; color: #4B5563; background: transparent; }
QPushButton#NavBtn:hover { background: #F3F4F6; }
QPushButton#NavBtn:checked { background: #EEF1FF; color: #4C6FFF; font-weight: 600; }
QFrame#Card { background: #FFFFFF; border: 1px solid #EEF0F4; border-radius: 16px; }
#StatValue { font-size: 30px; font-weight: 700; color: #1F2937; }
#StatLabel { color: #6B7280; }
#StatSub { color: #22C55E; font-size: 12px; }
#PageTitle { font-size: 24px; font-weight: 700; }
#PageSubtitle { color: #6B7280; }
QPushButton#Primary { background: #4C6FFF; color: white; border: none;
    border-radius: 10px; padding: 10px 16px; font-weight: 600; }
QPushButton#Primary:hover { background: #3B5BFF; }
QPushButton { background: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 10px; padding: 9px 14px; }
QPushButton:hover { background: #F9FAFB; }
QLineEdit, QComboBox, QSpinBox { border: 1px solid #E5E7EB; border-radius: 8px;
    padding: 8px; background: white; }
QTableWidget { background: white; border: 1px solid #EEF0F4; border-radius: 12px;
    gridline-color: #F1F2F4; }
QHeaderView::section { background: #FFFFFF; color: #9CA3AF; font-size: 11px;
    border: none; border-bottom: 1px solid #EEF0F4; padding: 10px; }
QTableWidget::item { padding: 8px; }
QScrollArea { border: none; background: #F6F7F9; }
)qss";
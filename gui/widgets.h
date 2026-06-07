#pragma once
#include <QFrame>
#include <QWidget>
#include <QString>
#include <functional>

QFrame*  makeCard(QWidget* parent = nullptr);
QFrame*  makePanelCard(QWidget* parent = nullptr);
QFrame*  statCard(const QString& emoji, const QString& value,
                  const QString& label, const QString& trend,
                  const QString& accentBg, const QString& trendColor = "#16A34A");
QWidget* pill(const QString& text, const QString& status);
QWidget* pageHeader(const QString& title, const QString& subtitle);
QFrame*  moduleCard(const QString& emoji, const QString& title,
                    const QString& desc, const QString& accentBg,
                    std::function<void()> onOpen = nullptr);
QWidget* activityRow(const QString& emoji, const QString& text,
                     const QString& time);
QWidget* eventRow(const QString& dateLabel, const QString& name,
                  const QString& emoji);
QWidget* menuRow(const QString& emoji, const QString& mealType,
                 const QString& items);
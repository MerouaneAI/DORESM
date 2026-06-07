#pragma once
#include <QFrame>
#include <QWidget>
#include <QString>

QFrame*  makeCard(QWidget* parent = nullptr);
QFrame*  statCard(const QString& emoji, const QString& value,
                  const QString& label, const QString& sub, const QString& accentBg);
QWidget* pill(const QString& text, const QString& status);
QWidget* pageHeader(const QString& title, const QString& subtitle);
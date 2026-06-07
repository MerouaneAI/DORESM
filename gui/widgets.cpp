#include "gui/widgets.h"
#include <QVBoxLayout>
#include <QLabel>

QFrame* makeCard(QWidget* parent) {
    auto* f = new QFrame(parent);
    f->setObjectName("Card");
    return f;
}

QFrame* statCard(const QString& emoji, const QString& value,
                 const QString& label, const QString& sub, const QString& accentBg) {
    auto* card = makeCard();
    auto* lay = new QVBoxLayout(card);
    lay->setContentsMargins(18, 18, 18, 18);
    lay->setSpacing(6);

    auto* icon = new QLabel(emoji);
    icon->setFixedSize(40, 40);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background:" + accentBg + "; border-radius:12px; font-size:18px;");
    lay->addWidget(icon);

    auto* v = new QLabel(value); v->setObjectName("StatValue");
    auto* l = new QLabel(label); l->setObjectName("StatLabel");
    lay->addWidget(v);
    lay->addWidget(l);
    if (!sub.isEmpty()) { auto* s = new QLabel(sub); s->setObjectName("StatSub"); lay->addWidget(s); }
    return card;
}

QWidget* pill(const QString& text, const QString& status) {
    auto* l = new QLabel(text);
    QString bg = "#EEF1FF", fg = "#4C6FFF";
    if (status == "Full" || status == "Maintenance") { bg = "#FEE2E2"; fg = "#EF4444"; }
    else if (status == "Available") { bg = "#DCFCE7"; fg = "#16A34A"; }
    else if (status == "Partial")   { bg = "#DBEAFE"; fg = "#2563EB"; }
    l->setStyleSheet("background:" + bg + "; color:" + fg +
                     "; border-radius:10px; padding:3px 10px; font-size:12px;");
    l->setAlignment(Qt::AlignCenter);
    return l;
}

QWidget* pageHeader(const QString& title, const QString& subtitle) {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 8);
    v->setSpacing(2);
    auto* t = new QLabel(title);    t->setObjectName("PageTitle");
    auto* s = new QLabel(subtitle); s->setObjectName("PageSubtitle");
    v->addWidget(t);
    v->addWidget(s);
    return w;
}
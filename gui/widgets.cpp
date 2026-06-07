#include "gui/widgets.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

QFrame* makeCard(QWidget* parent) {
    auto* f = new QFrame(parent);
    f->setObjectName("Card");
    return f;
}

QFrame* makePanelCard(QWidget* parent) {
    auto* f = new QFrame(parent);
    f->setObjectName("PanelCard");
    return f;
}

QFrame* statCard(const QString& emoji, const QString& value,
                 const QString& label, const QString& trend,
                 const QString& accentBg, const QString& trendColor) {
    auto* card = makeCard();
    auto* lay  = new QVBoxLayout(card);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(8);

    // Icon badge
    auto* icon = new QLabel(emoji);
    icon->setFixedSize(44, 44);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet(
        "background:" + accentBg + "; border-radius: 12px; font-size: 20px;");
    lay->addWidget(icon);

    // Value
    auto* val = new QLabel(value);
    val->setObjectName("StatValue");
    lay->addWidget(val);

    // Label
    auto* lbl = new QLabel(label);
    lbl->setObjectName("StatLabel");
    lay->addWidget(lbl);

    // Trend
    if (!trend.isEmpty()) {
        auto* tr = new QLabel(trend);
        tr->setObjectName("StatTrend");
        tr->setStyleSheet("color:" + trendColor + "; font-size:12px;");
        lay->addWidget(tr);
    }
    lay->addStretch();
    return card;
}

QWidget* pill(const QString& text, const QString& status) {
    auto* l = new QLabel(" " + text + " ");
    QString bg = "#EEF1FF", fg = "#4C6FFF";
    if (status == "Full" || status == "Maintenance") { bg = "#FEE2E2"; fg = "#EF4444"; }
    else if (status == "Available") { bg = "#DCFCE7"; fg = "#16A34A"; }
    else if (status == "Partial")   { bg = "#DBEAFE"; fg = "#2563EB"; }
    l->setStyleSheet("background:" + bg + "; color:" + fg +
                     "; border-radius:10px; padding:3px 8px;"
                     " font-size:12px; font-weight:500;");
    l->setAlignment(Qt::AlignCenter);
    return l;
}

QWidget* pageHeader(const QString& title, const QString& subtitle) {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(2);
    auto* t = new QLabel(title);    t->setObjectName("PageTitle");
    auto* s = new QLabel(subtitle); s->setObjectName("PageSubtitle");
    v->addWidget(t);
    v->addWidget(s);
    return w;
}

QFrame* moduleCard(const QString& emoji, const QString& title,
                   const QString& desc, const QString& accentBg,
                   std::function<void()> onOpen) {
    auto* card = new QFrame;
    card->setObjectName("ModuleCard");
    auto* v = new QVBoxLayout(card);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(8);

    auto* icon = new QLabel(emoji);
    icon->setFixedSize(44, 44);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background:" + accentBg +
                        "; border-radius:12px; font-size:20px;");
    v->addWidget(icon);

    auto* t = new QLabel(title); t->setObjectName("ModuleTitle");
    auto* d = new QLabel(desc);  d->setObjectName("ModuleDesc");
    d->setWordWrap(true);
    v->addWidget(t);
    v->addWidget(d);
    v->addStretch();

    auto* openBtn = new QPushButton("Open →");
    openBtn->setObjectName("ModuleOpen");
    openBtn->setStyleSheet(
        "QPushButton { background:transparent; border:none; color:#4C6FFF;"
        "font-size:12px; font-weight:500; text-align:left; padding:0; }"
        "QPushButton:hover { color:#3B5FFF; }");
    openBtn->setCursor(Qt::PointingHandCursor);
    if (onOpen) QObject::connect(openBtn, &QPushButton::clicked, openBtn, [onOpen]{ onOpen(); });
    v->addWidget(openBtn);
    return card;
}

QWidget* activityRow(const QString& emoji, const QString& text, const QString& time) {
    auto* w = new QWidget;
    auto* h = new QHBoxLayout(w);
    h->setContentsMargins(16, 12, 16, 12);
    h->setSpacing(12);

    auto* ic = new QLabel(emoji);
    ic->setFixedSize(34, 34);
    ic->setAlignment(Qt::AlignCenter);
    ic->setStyleSheet("background:#F3F4F8; border-radius:10px; font-size:16px;");
    h->addWidget(ic);

    auto* txt = new QLabel(text);
    txt->setObjectName("ActivityText");
    txt->setWordWrap(true);
    h->addWidget(txt, 1);

    auto* tm = new QLabel(time);
    tm->setObjectName("ActivityTime");
    tm->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    h->addWidget(tm);

    w->setStyleSheet("border-bottom: 1px solid #F3F4F8;");
    return w;
}

QWidget* eventRow(const QString& dateLabel, const QString& name, const QString& emoji) {
    auto* w = new QWidget;
    auto* h = new QHBoxLayout(w);
    h->setContentsMargins(0, 6, 0, 6);
    h->setSpacing(12);

    auto* ic = new QLabel(emoji);
    ic->setFixedSize(34, 34);
    ic->setAlignment(Qt::AlignCenter);
    ic->setStyleSheet("background:#F3F4F8; border-radius:10px; font-size:16px;");
    h->addWidget(ic);

    auto* col = new QWidget;
    auto* cv  = new QVBoxLayout(col);
    cv->setContentsMargins(0, 0, 0, 0); cv->setSpacing(1);
    auto* dt = new QLabel(dateLabel); dt->setObjectName("EventDate");
    auto* nm = new QLabel(name);      nm->setObjectName("EventName");
    cv->addWidget(dt);
    cv->addWidget(nm);
    h->addWidget(col, 1);
    return w;
}

QWidget* menuRow(const QString& emoji, const QString& mealType, const QString& items) {
    auto* w = new QWidget;
    auto* h = new QHBoxLayout(w);
    h->setContentsMargins(0, 4, 0, 4);
    h->setSpacing(8);
    auto* ic = new QLabel(emoji);
    ic->setStyleSheet("font-size:14px;");
    auto* lbl = new QLabel("<b>" + mealType + ":</b> " + items);
    lbl->setObjectName("MenuItem");
    h->addWidget(ic);
    h->addWidget(lbl, 1);
    return w;
}
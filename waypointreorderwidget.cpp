#include "waypointreorderwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>

static constexpr int CARD_HEIGHT  = 48;
static constexpr int CARD_SPACING =  6;
\
WaypointCard::WaypointCard(int index, const QString &name, QWidget *parent)
    : QWidget(parent), m_index(index), m_name(name)
{
    setFixedHeight(CARD_HEIGHT);  // forward-declared constant
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setCursor(index == 0 ? Qt::ArrowCursor : Qt::OpenHandCursor);
    setMouseTracking(true);
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(10, 0, 10, 0);
    lay->setSpacing(6);
    m_gripLabel = new QLabel("⠿", this);
    m_gripLabel->setFixedWidth(16);
    m_gripLabel->setStyleSheet(
        index == 0
            ? "color: transparent; font-size: 16px;"
            : "color: rgba(255,255,255,140); font-size: 16px;"
        );

    // Badge showing stop number
    m_indexLabel = new QLabel(this);
    m_indexLabel->setFixedSize(26, 26);
    m_indexLabel->setAlignment(Qt::AlignCenter);

    // Building name
    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setStyleSheet("color: white; font-size: 11px; font-weight: 600;");
    m_nameLabel->setWordWrap(false);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    lay->addWidget(m_gripLabel);
    lay->addWidget(m_indexLabel);
    lay->addWidget(m_nameLabel, 1);

    setLogicalIndex(index);
}

void WaypointCard::setLogicalIndex(int i)
{
    m_index = i;

    QString badgeStyle;
    QString badgeText;
    if (i == 0) {
        badgeStyle = "background-color: #800000; color: white; border-radius: 13px;"
                     "font-size: 9px; font-weight: bold;";
        badgeText  = "起点";
    } else {
        badgeStyle = QString("background-color: #1a5f7a; color: white; border-radius: 13px;"
                             "font-size: 11px; font-weight: bold;");
        badgeText  = QString::number(i);
    }
    m_indexLabel->setStyleSheet(badgeStyle);
    m_indexLabel->setText(badgeText);
}

void WaypointCard::setDragging(bool d)
{
    m_dragging = d;
    setCursor(d ? Qt::ClosedHandCursor : (m_index == 0 ? Qt::ArrowCursor : Qt::OpenHandCursor));
    update();
}

int WaypointCard::centreY() const
{
    return geometry().center().y();
}

void WaypointCard::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor bg;
    if (m_index == 0)        bg = QColor(80, 0, 0, 200);
    else if (m_dragging)     bg = QColor(26, 95, 122, 230);
    else                     bg = QColor(30, 40, 55, 200);

    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1,1,-1,-1), 8, 8);
    p.fillPath(path, bg);

    if (m_dragging) {
        // glowing border while dragging
        p.setPen(QPen(QColor(100, 200, 255, 180), 1.5));
        p.drawPath(path);
    } else {
        p.setPen(QPen(QColor(255, 255, 255, 25), 1));
        p.drawPath(path);
    }
}

void WaypointCard::mousePressEvent(QMouseEvent *event)
{
    if (m_index == 0) return;   // start point is locked
    if (event->button() == Qt::LeftButton) {
        m_dragStart = event->globalPos();
        setDragging(true);
        raise();
    }
}

void WaypointCard::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging) return;
    emit dragMoved(this, event->globalPos().y());
}

void WaypointCard::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_dragging) return;
    Q_UNUSED(event);
    setDragging(false);
    emit dragFinished(this);
}

WaypointReorderWidget::WaypointReorderWidget(const QVector<QPointF> &waypoints,
                                             const QStringList      &names,
                                             QWidget                *parent)
    : QWidget(parent),
    m_waypoints(waypoints),
    m_names(names)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    QHBoxLayout *outerLay = new QHBoxLayout(this);
    outerLay->setContentsMargins(12, 12, 0, 12);
    outerLay->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_panel = new QWidget(this);
    m_panel->setFixedWidth(PANEL_WIDTH);
    m_panel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    outerLay->addWidget(m_panel);
    outerLay->addStretch(1);

    QVBoxLayout *panelLay = new QVBoxLayout(m_panel);
    panelLay->setContentsMargins(PANEL_PAD, PANEL_PAD, PANEL_PAD, PANEL_PAD);
    panelLay->setSpacing(8);

    m_titleLabel = new QLabel("调整途经顺序", m_panel);
    m_titleLabel->setStyleSheet(
        "color: white; font-size: 13px; font-weight: bold;"
        "letter-spacing: 1px;"
        );
    m_titleLabel->setAlignment(Qt::AlignLeft);
    panelLay->addWidget(m_titleLabel);

    QLabel *hintLabel = new QLabel("拖动卡片以调整顺序", m_panel);
    hintLabel->setStyleSheet("color: rgba(255,255,255,140); font-size: 10px;");
    panelLay->addWidget(hintLabel);

    m_listContainer = new QWidget(m_panel);
    m_listContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    panelLay->addWidget(m_listContainer);

    QHBoxLayout *btnLay = new QHBoxLayout;
    btnLay->setSpacing(8);

    m_confirmBtn = new QPushButton("确认路线", m_panel);
    m_confirmBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #800000; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 8px 0; font-size: 12px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #a00000; }"
        "QPushButton:pressed { background-color: #600000; }"
        );

    m_cancelBtn = new QPushButton("取消", m_panel);
    m_cancelBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(255,255,255,30); color: white;"
        "  border: 1px solid rgba(255,255,255,60); border-radius: 6px;"
        "  padding: 8px 0; font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: rgba(255,255,255,55); }"
        );

    btnLay->addWidget(m_cancelBtn);
    btnLay->addWidget(m_confirmBtn);
    panelLay->addLayout(btnLay);

    connect(m_confirmBtn, &QPushButton::clicked, this, &WaypointReorderWidget::onConfirmClicked);
    connect(m_cancelBtn,  &QPushButton::clicked, this, &WaypointReorderWidget::onCancelClicked);

    rebuildCards();

    if (parent) {
        resize(parent->size());
    }
}

void WaypointReorderWidget::anchorToParent()
{
    if (parentWidget()) {
        resize(parentWidget()->size());
    }
}

void WaypointReorderWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void WaypointReorderWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(0, 0, 0, 80));

    QRect panelRect = m_panel->geometry();
    QPainterPath panelPath;
    panelPath.addRoundedRect(panelRect.adjusted(-2, -2, 2, 2), 14, 14);
    p.fillPath(panelPath, QColor(15, 22, 35, 220));

    p.setPen(QPen(QColor(255, 255, 255, 30), 1));
    p.drawPath(panelPath);
}

void WaypointReorderWidget::rebuildCards()
{
    for (WaypointCard *c : m_cards) c->deleteLater();
    m_cards.clear();

    for (int i = 0; i < m_names.size(); ++i) {
        WaypointCard *card = new WaypointCard(i, m_names[i], m_listContainer);
        connect(card, &WaypointCard::dragMoved,    this, &WaypointReorderWidget::onDragMoved);
        connect(card, &WaypointCard::dragFinished, this, &WaypointReorderWidget::onDragFinished);
        m_cards.append(card);
        card->show();
    }

    relayoutCards();

    int totalH = m_cards.size() * (CARD_HEIGHT + CARD_SPACING) - CARD_SPACING;
    m_listContainer->setFixedHeight(qMax(totalH, 10));
}

void WaypointReorderWidget::relayoutCards()
{
    int containerW = m_listContainer->width();
    if (containerW < 10) containerW = PANEL_WIDTH - 2 * PANEL_PAD;

    for (int i = 0; i < m_cards.size(); ++i) {
        int y = i * (CARD_HEIGHT + CARD_SPACING);
        m_cards[i]->setGeometry(0, y, containerW, CARD_HEIGHT);
        m_cards[i]->setLogicalIndex(i);
    }
}

void WaypointReorderWidget::onDragMoved(WaypointCard *card, int globalY)
{
    int localY = m_listContainer->mapFromGlobal(QPoint(0, globalY)).y();

    int hoveredSlot = localY / (CARD_HEIGHT + CARD_SPACING);
    hoveredSlot = qBound(1, hoveredSlot, m_cards.size() - 1);

    int currentSlot = m_cards.indexOf(card);
    if (currentSlot <= 0) return;

    if (hoveredSlot != currentSlot) {
        m_cards.remove(currentSlot);
        m_cards.insert(hoveredSlot, card);
        for (int i = 0; i < m_cards.size(); ++i) {
            if (m_cards[i] == card) continue;
            int y = i * (CARD_HEIGHT + CARD_SPACING);
            m_cards[i]->setGeometry(0, y, m_cards[i]->width(), CARD_HEIGHT);
            m_cards[i]->setLogicalIndex(i);
        }
    }
    int cardY = localY - CARD_HEIGHT / 2;
    int maxY  = m_listContainer->height() - CARD_HEIGHT;
    cardY = qBound(0, cardY, maxY);
    card->move(0, cardY);
    card->raise();
}

void WaypointReorderWidget::onDragFinished(WaypointCard *card)
{
    relayoutCards();
    QVector<QPointF> newWaypoints;
    QStringList      newNames;
    for (WaypointCard *c : m_cards) {
        int originalIdx = m_names.indexOf(c->name());
        if (originalIdx >= 0) {
            newWaypoints.append(m_waypoints[originalIdx]);
            newNames.append(m_names[originalIdx]);
        }
    }
    m_waypoints = newWaypoints;
    m_names     = newNames;
}

QVector<QPointF> WaypointReorderWidget::currentWaypoints() const
{
    return m_waypoints;
}

QStringList WaypointReorderWidget::currentNames() const
{
    return m_names;
}

void WaypointReorderWidget::onConfirmClicked()
{
    emit pathReordered(m_waypoints);
    hide();
}

void WaypointReorderWidget::onCancelClicked()
{
    emit reorderCancelled();
    hide();
}
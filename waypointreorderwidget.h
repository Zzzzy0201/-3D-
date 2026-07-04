#ifndef WAYPOINTREORDERWIDGET_H
#define WAYPOINTREORDERWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QStringList>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>

class WaypointCard : public QWidget
{
    Q_OBJECT
public:
    explicit WaypointCard(int index, const QString &name, QWidget *parent = nullptr);
    int  logicalIndex() const { return m_index; }
    void setLogicalIndex(int i);

    QString name() const { return m_name; }

    void setDragging(bool d);
    bool isDragging() const { return m_dragging; }

    int  centreY() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void dragMoved(WaypointCard *card, int globalY);
    void dragFinished(WaypointCard *card);

private:
    int     m_index;
    QString m_name;
    bool    m_dragging  = false;
    QPoint  m_dragStart;
    QLabel       *m_indexLabel;
    QLabel       *m_nameLabel;
    QLabel       *m_gripLabel;
};

class WaypointReorderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaypointReorderWidget(const QVector<QPointF>  &waypoints,
                                   const QStringList        &names,
                                   QWidget                  *parent = nullptr);

    void anchorToParent();
    QVector<QPointF>  currentWaypoints() const;
    QStringList       currentNames()     const;

signals:
    void pathReordered(const QVector<QPointF> &newPath);
    void reorderCancelled();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent  *event) override;

private slots:
    void onDragMoved(WaypointCard *card, int globalY);
    void onDragFinished(WaypointCard *card);
    void onConfirmClicked();
    void onCancelClicked();

private:
    void rebuildCards();
    void relayoutCards();
    void swapCards(int indexA, int indexB);
    QVector<QPointF>  m_waypoints;
    QStringList       m_names;
    QWidget      *m_panel;
    QWidget      *m_listContainer;
    QPushButton  *m_confirmBtn;
    QPushButton  *m_cancelBtn;
    QLabel       *m_titleLabel;

    QVector<WaypointCard *> m_cards;
    static constexpr int PANEL_WIDTH  = 260;
    static constexpr int PANEL_PAD    =  14;
};

#endif // WAYPOINTREORDERWIDGET_H
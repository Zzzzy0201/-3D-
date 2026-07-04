#ifndef SMALL_MAPWIDGET_H
#define SMALL_MAPWIDGET_H

#include <QWidget>
#include <QPixmap>

class ThumbnailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ThumbnailWidget(QWidget *parent = nullptr);
    void setMapImage(const QPixmap &pixmap);
    void setViewport(double zoom, const QPointF &offset, int viewWidth, int viewHeight);

signals:
    void viewportRequested(const QPointF &center); // 请求移动主视图到该点（原始图片坐标）

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QPixmap m_fullImage;
    QRectF m_viewRect;
    QPointF mapToImage(const QPoint &pos) const;
};

#endif // SMALL_MAPWIDGET_H

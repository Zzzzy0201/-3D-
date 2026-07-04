#include "small_mapwidget.h"
#include <QPainter>
#include <QMouseEvent>

ThumbnailWidget::ThumbnailWidget(QWidget *parent) : QWidget(parent)
{
    setFixedSize(200, 200); //临时尺寸
}

void ThumbnailWidget::setMapImage(const QPixmap &pixmap)
{
    m_fullImage = pixmap;
    // 调整缩略图大小
    if (!m_fullImage.isNull()) {
        int w = 200;
        int h = w * m_fullImage.height() / m_fullImage.width();
        setFixedSize(w, h);
    }
    update();
}

void ThumbnailWidget::setViewport(double zoom, const QPointF &offset, int viewWidth, int viewHeight)
{
    if (m_fullImage.isNull()) return;
    double w = viewWidth / zoom;
    double h = viewHeight / zoom;
    m_viewRect = QRectF(offset.x(), offset.y(), w, h);
    m_viewRect = m_viewRect.intersected(QRectF(0, 0, m_fullImage.width(), m_fullImage.height()));
    update();
}

void ThumbnailWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    if (m_fullImage.isNull()) return;

    // 绘制缩略图
    QRect targetRect = rect();
    QPixmap thumb = m_fullImage.scaled(targetRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap(targetRect, thumb, thumb.rect());

    // 绘制区缩略图
    QSizeF thumbSize = thumb.size();
    QRectF thumbDrawRect((targetRect.width() - thumbSize.width()) / 2,
                         (targetRect.height() - thumbSize.height()) / 2,
                         thumbSize.width(), thumbSize.height());

    double scaleX = thumbDrawRect.width() / m_fullImage.width();
    double scaleY = thumbDrawRect.height() / m_fullImage.height();
    QRectF viewOnThumb(m_viewRect.x() * scaleX + thumbDrawRect.x(),
                       m_viewRect.y() * scaleY + thumbDrawRect.y(),
                       m_viewRect.width() * scaleX,
                       m_viewRect.height() * scaleY);

    painter.setPen(QPen(Qt::red, 2));
    painter.drawRect(viewOnThumb);
}

void ThumbnailWidget::mousePressEvent(QMouseEvent *event)
{
    QPointF imgPos = mapToImage(event->pos());
    if (imgPos.x() >= 0 && imgPos.x() <= m_fullImage.width() && imgPos.y() >= 0 && imgPos.y() <= m_fullImage.height()) {
        emit viewportRequested(imgPos);
    }
}

QPointF ThumbnailWidget::mapToImage(const QPoint &pos) const
{
    if (m_fullImage.isNull()) return QPointF(-1,-1);
    QRect targetRect = rect();
    QPixmap thumb = m_fullImage.scaled(targetRect.size(), Qt::KeepAspectRatio);
    QSizeF thumbSize = thumb.size();
    QRectF thumbRect((targetRect.width() - thumbSize.width()) / 2,
                     (targetRect.height() - thumbSize.height()) / 2,
                     thumbSize.width(), thumbSize.height());
    double x = (pos.x() - thumbRect.x()) / thumbRect.width() * m_fullImage.width();
    double y = (pos.y() - thumbRect.y()) / thumbRect.height() * m_fullImage.height();
    return QPointF(x, y);
}
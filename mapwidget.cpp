#include "mapwidget.h"
#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMouseEvent>
#include <QToolTip>
#include <QEvent>

MapWidget::MapWidget(QWidget *parent) : QWidget(parent), transformValid(false), m_zoom(1.0), m_viewOffset(0,0), m_dragging(false), m_minZoom(0.2)
{
    setMinimumSize(400, 600);
    setMouseTracking(true);
}

bool MapWidget::loadMapImage(const QString &imagePath)
{
    if (mapImage.load(imagePath)) {
        emit mapImageLoaded();
        update();
        return true;
    }
    return false;
}

bool MapWidget::loadBuildings(const QString &csvPath)
{
    QFile file(csvPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    allBuildings.clear();
    QString header = in.readLine();

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');
        if (fields.size() >= 5) {
            BuildingInfo info;
            info.name = fields[0].trimmed();
            info.lon = fields[1].toDouble();
            info.lat = fields[2].toDouble();
            info.stayTime = fields[3].toInt();
            QString tagsStr = fields[4].trimmed();
            if (!tagsStr.isEmpty())
                info.tags = tagsStr.split(';', Qt::SkipEmptyParts);
            allBuildings.append(info);
        } else if (fields.size() >= 3) {
            //停留时间默认0，标签空
            BuildingInfo info;
            info.name = fields[0].trimmed();
            info.lon = fields[1].toDouble();
            info.lat = fields[2].toDouble();
            info.stayTime = 0;
            allBuildings.append(info);
        }
    }
    file.close();
    return true;
}

void MapWidget::addReferencePoint(double lon, double lat, double px, double py)
{
    transformer.addReferencePoint(lon, lat, px, py);
}

bool MapWidget::computeTransform()
{
    if (!transformer.compute())
        return false;
    transformValid = true;
    buildings.clear();
    for (const auto &b : allBuildings) {
        QPointF pxPos = transformer.lonLatToPx(b.lon, b.lat);
        // 检查坐标是否在图片范围内
        if (pxPos.x() >= 0 && pxPos.x() < mapImage.width() &&
            pxPos.y() >= 0 && pxPos.y() < mapImage.height()) {
            BuildingInfo valid = b;
            valid.pxPos = pxPos;
            buildings.append(valid);
        }
    }
    update();
    return true;
}

void MapWidget::fitToWindow()
{
    if (mapImage.isNull()) return;
    double scaleX = (double)width() / mapImage.width();
    double scaleY = (double)height() / mapImage.height();
    m_minZoom = qMin(scaleX, scaleY);
    m_zoom = m_minZoom;
    double viewWidth = width() / m_zoom;
    double viewHeight = height() / m_zoom;
    m_viewOffset.setX((mapImage.width() - viewWidth) / 2);
    m_viewOffset.setY((mapImage.height() - viewHeight) / 2);
    clampViewOffset();
    update();
    emit viewportChanged(m_zoom, m_viewOffset, width(), height());
}

void MapWidget::paintEvent(QPaintEvent *)
{
    //绘制背景
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    if (mapImage.isNull()) return;

    QRectF sourceRect(m_viewOffset.x(), m_viewOffset.y(), width() / m_zoom, height() / m_zoom);
    sourceRect = sourceRect.intersected(QRectF(0, 0, mapImage.width(), mapImage.height()));

    QRect targetRect(0, 0, width(), height());
    painter.drawPixmap(targetRect, mapImage, sourceRect);

    if (!transformValid) return;

    //绘制建筑圆点
    for (const auto &b : buildings) {
        QPointF imgPos = b.pxPos;
        bool isHighlight = highlightedBuildings.contains(b.name);
        bool isSelected = selectedBuildings.contains(b.name);
        bool isRecommended = recommendedBuildings.contains(b.name);
        bool inViewport = sourceRect.contains(imgPos);

        if (!inViewport && !isHighlight && !isRecommended) {
            continue;
        }
        QPointF widgetPos = (imgPos - m_viewOffset) * m_zoom;

        if (isHighlight) {
            painter.setPen(Qt::red);
            painter.setBrush(QColor(255, 0, 0, 200));
            painter.drawEllipse(widgetPos, 8, 8);
            painter.setPen(Qt::white);
            QFont font = painter.font();
            font.setBold(true);
            font.setPointSize(9);
            painter.setFont(font);
            painter.drawText(widgetPos + QPointF(12, -12), b.name);
        } else if (isSelected) {
            painter.setPen(Qt::green);
            painter.setBrush(QColor(0, 255, 0, 200));
            painter.drawEllipse(widgetPos, 6, 6);
        } else if (isRecommended) {
            painter.setPen(Qt::blue);
            painter.setBrush(QColor(0, 0, 255, 150));
            painter.drawEllipse(widgetPos, 7, 7);
            painter.setPen(Qt::white);
            painter.drawText(widgetPos + QPointF(10, -10), b.name);
        }else {
            painter.setPen(Qt::white);
            painter.setBrush(QColor(0, 0, 0, 100));
            painter.drawEllipse(widgetPos, 4, 4);
        }
    }

    // 绘制路线
    if (m_routePoints.size() >= 2) {
        QPen pen(Qt::red, 3);
        painter.setPen(pen);
        QVector<QPointF> widgetPoints;
        for (const QPointF& pt : m_routePoints) {
            QPointF widgetPos = (pt - m_viewOffset) * m_zoom;
            widgetPoints.append(widgetPos);
        }
        for (int i = 0; i < widgetPoints.size() - 1; ++i) {
            painter.drawLine(widgetPoints[i], widgetPoints[i+1]);
        }
    }
}

void MapWidget::wheelEvent(QWheelEvent *event)
{
    double factor = 1.1;
    double newZoom = m_zoom;
    if (event->angleDelta().y() > 0)
        newZoom *= factor;
    else
        newZoom /= factor;

    // 限制缩放范围
    newZoom = qMax(m_minZoom, qMin(5.0, newZoom));
    if (qFabs(newZoom - m_zoom) < 1e-6)
        return;

    QPointF mousePos = event->position();
    QPointF imgPos = m_viewOffset + QPointF(mousePos.x() / m_zoom, mousePos.y() / m_zoom);

    m_zoom = newZoom;
    m_viewOffset = imgPos - QPointF(mousePos.x() / m_zoom, mousePos.y() / m_zoom);
    clampViewOffset();
    update();
    emit viewportChanged(m_zoom, m_viewOffset, width(), height());
}

void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF imgPos = m_viewOffset + QPointF(event->pos().x() / m_zoom, event->pos().y() / m_zoom);
        const double tolerance = 10.0;
        bool hit = false;
        QString hitName;
        for (const auto &b : buildings) {
            double dx = imgPos.x() - b.pxPos.x();
            double dy = imgPos.y() - b.pxPos.y();
            if (dx*dx + dy*dy <= tolerance*tolerance) {
                hit = true;
                hitName = b.name;
                break;
            }
        }
        if (hit) {
            if (highlightedBuildings.contains(hitName)) {
                emit highlightBuildingClicked(hitName);
            } else {
                emit buildingClicked(hitName, QPointF());
            }
            return;
        }
        m_dragging = true;
        m_lastDragPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QWidget::mousePressEvent(event);
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        QPoint delta = event->pos() - m_lastDragPos;
        QPointF deltaImg(delta.x() / m_zoom, delta.y() / m_zoom);
        m_viewOffset -= deltaImg;
        clampViewOffset();
        m_lastDragPos = event->pos();
        update();
        emit viewportChanged(m_zoom, m_viewOffset, width(), height());
        return;
    }
    //悬停
    QPoint mousePos = event->pos();
    QPointF imgPos = m_viewOffset + QPointF(mousePos.x() / m_zoom, mousePos.y() / m_zoom);
    const double tolerance = 10.0;
    QString hoverName;
    for (const auto &b : buildings) {
        double dx = imgPos.x() - b.pxPos.x();
        double dy = imgPos.y() - b.pxPos.y();
        if (dx*dx + dy*dy <= tolerance*tolerance) {
            hoverName = b.name;
            break;
        }
    }

    if (!hoverName.isEmpty()) {
        QToolTip::showText(event->globalPosition().toPoint(), hoverName, this);
    } else {
        QToolTip::hideText();
    }

    QWidget::mouseMoveEvent(event);
}

void MapWidget::leaveEvent(QEvent *event)
{
    QToolTip::hideText();
    QWidget::leaveEvent(event);
}

void MapWidget::clampViewOffset()
{
    if (mapImage.isNull()) return;
    double viewW = width() / m_zoom;
    double viewH = height() / m_zoom;
    double maxX = mapImage.width() - viewW;
    double maxY = mapImage.height() - viewH;

    if (maxX < 0) {
        m_viewOffset.setX((mapImage.width() - viewW) / 2);
    } else {
        m_viewOffset.setX(qMax(0.0, qMin(m_viewOffset.x(), maxX)));
    }

    if (maxY < 0) {
        m_viewOffset.setY((mapImage.height() - viewH) / 2);
    } else {
        m_viewOffset.setY(qMax(0.0, qMin(m_viewOffset.y(), maxY)));
    }
}

void MapWidget::moveToCenter(const QPointF &centerImgPos)
{
    m_viewOffset = centerImgPos - QPointF(width() / m_zoom / 2, height() / m_zoom / 2);
    clampViewOffset();
    update();
    emit viewportChanged(m_zoom, m_viewOffset, width(), height());
}

void MapWidget::selectBuilding(const QString &name)
{
    if (!selectedBuildings.contains(name)) {
        selectedBuildings.insert(name);
        update();
    }
}

void MapWidget::deselectBuilding(const QString &name)
{
    if (selectedBuildings.remove(name)) {
        update();
    }
}

QStringList MapWidget::getAllBuildingNames() const
{
    QStringList names;
    for (const auto &b : buildings) {
        names << b.name;
    }
    return names;
}

void MapWidget::highlightBuilding(const QString &name)
{
    QPointF imgPos = getBuildingImgPos(name);
    if (imgPos.x() >= 0) {
        moveToCenter(imgPos);
    }
    highlightedBuildings.clear();
    highlightedBuildings.insert(name);
    update();
}

void MapWidget::clearHighlight()
{
    highlightedBuildings.clear();
    update();
}

QPointF MapWidget::getBuildingImgPos(const QString &name) const
{
    for (const auto &b : buildings) {
        if (b.name == name) {
            return b.pxPos;
        }
    }
    return QPointF(-1, -1);
}

QPointF MapWidget::getBuildingWidgetPos(const QString &name) const
{
    QPointF imgPos = getBuildingImgPos(name);
    if (imgPos.x() < 0) return QPointF(-1, -1);
    QPointF widgetPos = (imgPos - m_viewOffset) * m_zoom;
    return widgetPos;
}

void MapWidget::setRecommendedBuildings(const QStringList &names)
{
    recommendedBuildings.clear();
    for (const QString &name : names) {
        recommendedBuildings.insert(name);
    }
    update();
}

void MapWidget::setRoutePoints(const QVector<QPointF>& points)
{
    m_routePoints = points;
    update();
}

QPointF MapWidget::getBuildingPxPos(const QString& name) const
{
    for (const auto& b : buildings) {
        if (b.name == name) return b.pxPos;
    }
    return QPointF(-1, -1);
}

QPointF MapWidget::lonLatToPx(double lon, double lat) const
{
    return transformer.lonLatToPx(lon, lat);
}
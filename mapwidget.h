#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QVector>
#include <QPointF>
#include <QSet>
#include "maptransformer.h"

struct BuildingInfo {
    QString name;
    double lon;
    double lat;
    QPointF pxPos;
    int type;
    double stayTime;
    QStringList tags;
};

class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);
    bool loadMapImage(const QString &imagePath);
    bool loadBuildings(const QString &csvPath);
    void addReferencePoint(double lon, double lat, double px, double py);
    bool computeTransform();
    void fitToWindow();
    QSize getMapImageSize() const { return mapImage.size(); }
    QPixmap getMapImage() const { return mapImage; }
    void moveToCenter(const QPointF &centerImgPos);
    void selectBuilding(const QString &name);
    void deselectBuilding(const QString &name);
    QStringList getAllBuildingNames() const;
    void highlightBuilding(const QString &name);
    void clearHighlight();
    QPointF getBuildingImgPos(const QString &name) const;
    QPointF getBuildingWidgetPos(const QString &name) const;
    void setRecommendedBuildings(const QStringList &names);
    QVector<BuildingInfo> getAllBuildings() const { return allBuildings; }
    QVector<BuildingInfo> getBuildings() const { return buildings; }
    void setRoutePoints(const QVector<QPointF>& points);
    QPointF getBuildingPxPos(const QString& name) const;
    QPointF lonLatToPx(double lon, double lat) const;

signals:
    void mapImageLoaded();
    void viewportChanged(double zoom, const QPointF &offset, int viewWidth, int viewHeight);
    void buildingClicked(const QString &name,const QPointF &pxPos);
    void highlightBuildingClicked(const QString &name);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QPixmap mapImage;
    QVector<BuildingInfo> buildings;    // 有效建筑
    QVector<BuildingInfo> allBuildings;   // 未过滤建筑
    MapTransformer transformer;
    bool transformValid;
    double m_zoom;
    QPointF m_viewOffset;
    QPoint m_lastDragPos;
    bool m_dragging;
    double m_minZoom;
    void clampViewOffset();
    QPointF mapToWidget(const QPointF &imgPoint) const;
    QPointF widgetToMap(const QPoint &widgetPoint) const;
    QSet<QString> selectedBuildings;
    QSet<QString> highlightedBuildings;
    QSet<QString> recommendedBuildings;
    QVector<QPointF> m_routePoints;
};

#endif // MAPWIDGET_H
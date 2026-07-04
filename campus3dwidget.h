#ifndef CAMPUS3DWIDGET_H
#define CAMPUS3DWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QList>
#include "building.h" // Includes our custom building data layout
#include "cameramanager.h"
#include "maploader.h"

class Campus3DWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit Campus3DWidget(QWidget *parent = nullptr);
    ~Campus3DWidget();

    QVector3D map2DTo3D(double mapX, double mapY, float height = 0.0f);

    bool loadBuildingsFromFile(const QString &filePath);

    void setNavigationPath(const QVector<QPointF>& pathLonLat);
    QVector<QPointF> getNavigationPathLonLat() const { return m_pathLonLat; }

    struct RoadNodeLonLat {
        int id;
        double lon;
        double lat;
    };
    struct RoadEdgeInfo {
        int from;
        int to;
        double length;
    };

    // 获取所有道路节点（包含经纬度）
    QVector<RoadNodeLonLat> getRoadNodesLonLat() const;
    // 获取所有道路边（包含长度）
    QVector<RoadEdgeInfo> getRoadEdgesInfo() const;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    double centerMapX;
    double centerMapY;

    std::vector<RoadNode> m_roadNodes;
    std::vector<RoadEdge> m_roadEdges;

    CameraManager camera;

    QList<BuildingData> campusBuildings;

    QList<QVector3D> activeNavigationPath;
    QPoint worldToScreen(const QVector3D& worldPos);
    QPoint worldToScreenWithMatrices(const QVector3D& worldPos,
                                     const GLdouble modelMatrix[16],
                                     const GLdouble projMatrix[16],
                                     const GLint viewport[4]);
    QPoint manualWorldToScreen(const QVector3D& worldPos);
    QVector<QPointF> m_pathLonLat;
    std::vector<RoadNode> m_originalRoadNodes;
    std::vector<BuildingFootprint> m_buildingFootprints;
    QVector<int> m_footprintIndexForBuilding;
    void buildFootprintMatching();
};

#endif // CAMPUS3DWIDGET_H
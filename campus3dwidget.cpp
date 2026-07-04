#include "campus3dwidget.h"
#include "gldrawingengine.h"
#include <cmath>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QSet>
#include <QMap>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

Campus3DWidget::Campus3DWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    centerMapX = 116.30393853823529;
    centerMapY = 39.99052945;

    activeNavigationPath.append(map2DTo3D(116.303938, 39.990529));
    activeNavigationPath.append(map2DTo3D(116.305000, 39.990529));
    activeNavigationPath.append(map2DTo3D(116.305000, 39.992637));
    activeNavigationPath.append(map2DTo3D(116.305783, 39.992637));

    bool success = loadBuildingsFromFile(":/data/buildings.txt");
    if (!success) {
        campusBuildings.append({"Fallback Library", 116.303938, 39.990529, 60.0f, 60.0f, 40.0f, 0.7f, 0.5f, 0.4f});
        campusBuildings.append({"Fallback Boya", 116.305783, 39.992637, 30.0f, 30.0f, 80.0f, 0.8f, 0.8f, 0.8f});
    }

    loadRoadNetwork(":/data/road_network.txt", m_roadNodes, m_roadEdges);
    loadBuildingFootprints(":/data/building_footprints.txt", m_buildingFootprints);
    buildFootprintMatching();
    m_originalRoadNodes = m_roadNodes;

    for (size_t i = 0; i < m_roadNodes.size(); ++i) {
        float scaledX = static_cast<float>((m_roadNodes[i].x - centerMapX) * 111000.0f);
        float scaledZ = -static_cast<float>((m_roadNodes[i].z - centerMapY) * 111000.0f);

        m_roadNodes[i].x = scaledX;
        m_roadNodes[i].y = 0.0f; // Flat on the grid floor
        m_roadNodes[i].z = scaledZ;
    }

    update();
}

Campus3DWidget::~Campus3DWidget() {}

QVector3D Campus3DWidget::map2DTo3D(double mapX, double mapY, float height)
{
    float x3d = static_cast<float>((mapX - centerMapX) * 111000.0f);
    float z3d = -static_cast<float>((mapY - centerMapY) * 111000.0f);
    return QVector3D(x3d, height, z3d);
}

void Campus3DWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.65f, 0.80f, 0.92f, 1.0f);
}

void Campus3DWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = static_cast<float>(w) / static_cast<float>(h);
    float top = 1.0f * tanf(45.0f * 0.5f * M_PI / 180.0f);
    glFrustum(-top * aspect, top * aspect, -top, top, 1.0f, 2000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void Campus3DWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = width() / (float)height();
    float top = 1.0f * tanf(45.0f * 0.5f * M_PI / 180.0f);
    glFrustum(-top * aspect, top * aspect, -top, top, 1.0f, 2000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.applyTransformations();

    GLDrawingEngine::drawGroundPlane(this, 5000.0f, -0.05f, 0.35f, 0.55f, 0.35f);
    {
        static const double lakeLonLat[][2] = {
            {116.3035094, 39.993043},
            {116.3033537, 39.9931067},
            {116.3031629, 39.9931869},
            {116.3030625, 39.9931745},
            {116.3029589, 39.993304},
            {116.3028302, 39.9935341},
            {116.3028682, 39.9936397},
            {116.3029329, 39.993667},
            {116.3029346, 39.9937117},
            {116.3029993, 39.9937303},
            {116.3030104, 39.9937511},
            {116.3029854, 39.9937695},
            {116.3028161, 39.9937587},
            {116.3027322, 39.9937508},
            {116.3026672, 39.9937482},
            {116.3025905, 39.9936942},
            {116.3025176, 39.9936193},
            {116.3024288, 39.9936053},
            {116.3023751, 39.9936007},
            {116.3023452, 39.9935697},
            {116.302344, 39.9935365},
            {116.3023882, 39.9935032},
            {116.3024828, 39.9934425},
            {116.3024896, 39.9933775},
            {116.3024472, 39.9933222},
            {116.3023549, 39.9933023},
            {116.302146, 39.9933631},
            {116.3018332, 39.9934889},
            {116.3013727, 39.9937049},
            {116.3011763, 39.9937128},
            {116.3009637, 39.9937071},
            {116.3009466, 39.9936881},
            {116.3009591, 39.9936665},
            {116.3012787, 39.9936641},
            {116.3014572, 39.9935969},
            {116.3016703, 39.9934361},
            {116.3019178, 39.9933113},
            {116.3022794, 39.9932277},
            {116.302224, 39.9930774},
            {116.3023058, 39.9929656},
            {116.3025791, 39.9928676},
            {116.3028988, 39.9927253},
            {116.3030987, 39.9926724},
            {116.3034079, 39.9926241},
            {116.3034224, 39.9924909},
            {116.3034271, 39.9922345},
            {116.3034, 39.9922064},
            {116.303374, 39.9921732},
            {116.3033799, 39.9921488},
            {116.3034061, 39.99211},
            {116.303444, 39.9920656},
            {116.3035205, 39.9920327},
            {116.3036092, 39.9920211},
            {116.3037169, 39.9920249},
            {116.3038013, 39.9920264},
            {116.3039008, 39.9920442},
            {116.3039694, 39.9920627},
            {116.3039929, 39.9920863},
            {116.3039935, 39.9921227},
            {116.3039795, 39.9921794},
            {116.3039461, 39.9922172},
            {116.3039113, 39.9922416},
            {116.3038672, 39.9922572},
            {116.3038189, 39.992272},
            {116.3037684, 39.992281},
            {116.3037073, 39.9922861},
            {116.3036392, 39.9922819},
            {116.3035199, 39.9926365},
            {116.3035856, 39.992657},
            {116.304006, 39.9928305},
            {116.3041462, 39.9928741},
            {116.3041786, 39.9928947},
            {116.3041947, 39.992916},
            {116.304192, 39.9929347},
            {116.3042026, 39.9929499},
            {116.304224, 39.9929587},
            {116.304252, 39.9929625},
            {116.3042815, 39.9929635},
            {116.3043119, 39.992954},
            {116.3043323, 39.9929388},
            {116.3043484, 39.9929167},
            {116.3043622, 39.9929039},
            {116.3043948, 39.9928963},
            {116.3044234, 39.9928874},
            {116.3044584, 39.992873},
            {116.3044883, 39.9928498},
            {116.3045132, 39.9928145},
            {116.3045457, 39.9927768},
            {116.3045865, 39.9927381},
            {116.3046217, 39.9927065},
            {116.3046504, 39.9926781},
            {116.304697, 39.9926603},
            {116.3047416, 39.992651},
            {116.3047989, 39.9926503},
            {116.3048613, 39.9926585},
            {116.3049132, 39.9926647},
            {116.3049939, 39.9926612},
            {116.305063, 39.992653},
            {116.3051162, 39.9926483},
            {116.3051566, 39.9926591},
            {116.3052043, 39.9926848},
            {116.3052487, 39.9927203},
            {116.3052816, 39.9927866},
            {116.305293, 39.9928397},
            {116.3053036, 39.9930063},
            {116.3052607, 39.9934913},
            {116.3051991, 39.993659},
            {116.3048723, 39.9940765},
            {116.3048394, 39.9941246},
            {116.304847, 39.9941673},
            {116.3048661, 39.994201},
            {116.3049176, 39.9942395},
            {116.3049891, 39.9942672},
            {116.3050553, 39.9942851},
            {116.3050529, 39.9943213},
            {116.3050228, 39.9943336},
            {116.3049646, 39.9943475},
            {116.3049366, 39.9943491},
            {116.3048765, 39.9943021},
            {116.3048379, 39.9942763},
            {116.3047955, 39.9942257},
            {116.3046196, 39.9942187},
            {116.3037314, 39.9942048},
            {116.3033451, 39.9941588},
            {116.3031927, 39.9941078},
            {116.3031176, 39.9940338},
            {116.3031337, 39.9937543},
            {116.30319, 39.9936783},
            {116.3032705, 39.9936516},
            {116.3033885, 39.9936742},
            {116.3034673, 39.993739},
            {116.3035132, 39.9937471},
            {116.303599, 39.9937379},
            {116.303658, 39.9937297},
            {116.3037144, 39.9937369},
            {116.3037559, 39.993781},
            {116.3037993, 39.9938234},
            {116.3038538, 39.9938375},
            {116.3038954, 39.9938221},
            {116.3039008, 39.9937913},
            {116.3039008, 39.9937081},
            {116.3038485, 39.9936331},
            {116.3038268, 39.993574},
            {116.3038236, 39.9932887},
            {116.303775, 39.9931212},
            {116.3036541, 39.9930672}
        };
        QList<QVector3D> lakePoints;
        for (const auto& pt : lakeLonLat) {
            lakePoints.append(map2DTo3D(pt[0], pt[1]));
        }
        GLDrawingEngine::drawFilledArea(this, lakePoints, 0.01f, 0.25f, 0.45f, 0.70f);
    }

    for (int bi = 0; bi < campusBuildings.size(); ++bi) {
        const BuildingData& b = campusBuildings[bi];
        int fpIdx = (bi < m_footprintIndexForBuilding.size()) ? m_footprintIndexForBuilding[bi] : -1;

        if (fpIdx >= 0) {
            const BuildingFootprint& fp = m_buildingFootprints[fpIdx];

            QList<QVector3D> outer3D;
            outer3D.reserve(fp.outer.size());
            for (const QPointF& pt : fp.outer) {
                outer3D.append(map2DTo3D(pt.x(), pt.y()));
            }

            QList<QList<QVector3D>> holes3D;
            for (const QVector<QPointF>& hole : fp.holes) {
                QList<QVector3D> holeConverted;
                holeConverted.reserve(hole.size());
                for (const QPointF& pt : hole) {
                    holeConverted.append(map2DTo3D(pt.x(), pt.y()));
                }
                holes3D.append(holeConverted);
            }

            GLDrawingEngine::drawExtrudedFootprint(this, outer3D, holes3D, 0.0f, b.height, b.r, b.g, b.b);

        } else if (b.isCourtyard) {
            QVector3D position = map2DTo3D(b.longitude, b.latitude);
            GLDrawingEngine::drawCourtyard(this, position, b.width, b.length, b.height,
                                           b.r, b.g, b.b, b.openingDirection, b.wallThickness);
        } else {
            QVector3D position = map2DTo3D(b.longitude, b.latitude);
            GLDrawingEngine::drawCube(this, position, b.width, b.length, b.height, b.r, b.g, b.b);
        }
    }

    GLDrawingEngine::drawRoadsSolid(QOpenGLContext::currentContext()->functions(), m_roadNodes, m_roadEdges,
                                    6.0f, 0.05f, 0.55f, 0.55f, 0.58f);
    GLDrawingEngine::drawRoute(this, activeNavigationPath, 1.0f, 0.1f, 0.1f);

    GLdouble modelMatrix[16];
    GLdouble projMatrix[16];
    GLint viewport[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    const float dpr = static_cast<float>(devicePixelRatio());

    for (const BuildingData& b : campusBuildings) {
        QVector3D worldPos = map2DTo3D(b.longitude, b.latitude);

        worldPos.setY(b.height + 5.0f);  // +5 gives a small float gap above roof

        QPoint screenPos = worldToScreenWithMatrices(worldPos, modelMatrix, projMatrix, viewport);

        if (screenPos.x() < 0 || screenPos.y() < 0) continue;
        int logicalX = static_cast<int>(screenPos.x() / dpr);
        int logicalY = static_cast<int>(screenPos.y() / dpr);

        if (logicalX < 0 || logicalX > width() ||
            logicalY < 0 || logicalY > height()) continue;

        QFontMetrics metrics(font);
        int textWidth = metrics.horizontalAdvance(b.name);

        int renderX = logicalX - (textWidth / 2);
        int renderY = logicalY;

        painter.setPen(Qt::black);
        painter.drawText(renderX + 1, renderY + 1, b.name);
        painter.setPen(Qt::white);
        painter.drawText(renderX, renderY, b.name);
    }

    painter.end();
}

void Campus3DWidget::mousePressEvent(QMouseEvent *event) { camera.handleMousePress(event); }
void Campus3DWidget::mouseMoveEvent(QMouseEvent *event) { camera.handleMouseMove(event); update(); }
void Campus3DWidget::wheelEvent(QWheelEvent *event)     { camera.handleWheel(event);     update(); }

void Campus3DWidget::keyPressEvent(QKeyEvent *event)
{
    QOpenGLWidget::keyPressEvent(event);
}


QPoint Campus3DWidget::worldToScreenWithMatrices(const QVector3D& worldPos,
                                                 const GLdouble modelMatrix[16],
                                                 const GLdouble projMatrix[16],
                                                 const GLint viewport[4])
{
    double eyeZ = modelMatrix[2] * worldPos.x() +
                  modelMatrix[6] * worldPos.y() +
                  modelMatrix[10] * worldPos.z() +
                  modelMatrix[14];

    if (eyeZ >= -1.0) {
        return QPoint(-1, -1);
    }

    GLdouble screenX, screenY, screenZ;

    if (gluProject(worldPos.x(), worldPos.y(), worldPos.z(),
                   modelMatrix, projMatrix, viewport,
                   &screenX, &screenY, &screenZ)) {

        if (screenZ < 0.0 || screenZ > 1.0) {
            return QPoint(-1, -1);
        }

        int finalX = static_cast<int>(screenX);
        int finalY = viewport[3] - static_cast<int>(screenY);
        return QPoint(finalX, finalY);
    }

    return QPoint(-1, -1);
}

bool Campus3DWidget::loadBuildingsFromFile(const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Critical Error: Could not open campus data file at path:" << filePath;
        return false;
    }

    campusBuildings.clear();

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        QStringList tokens = line.split('|');
        if (tokens.size() < 9) {
            continue;
        }

        BuildingData data;
        data.name      = tokens[0];
        data.longitude = tokens[1].toDouble();
        data.latitude  = tokens[2].toDouble();
        data.width     = tokens[3].toFloat();
        data.length    = tokens[4].toFloat();
        data.height    = tokens[5].toFloat();
        data.r         = tokens[6].toFloat();
        data.g         = tokens[7].toFloat();
        data.b         = tokens[8].toFloat();

        campusBuildings.append(data);
    }

    file.close();
    {
        static const QSet<QString> leftColumn  = {"一院", "二院", "三院"};
        static const QSet<QString> rightColumn = {"四院", "五院", "六院"};
        for (BuildingData &b : campusBuildings) {
            if (leftColumn.contains(b.name)) {
                b.isCourtyard = true;
                b.openingDirection = 1; // 朝东
                b.wallThickness = 6.0f;
            } else if (rightColumn.contains(b.name)) {
                b.isCourtyard = true;
                b.openingDirection = 3; // 朝西
                b.wallThickness = 6.0f;
            }
        }
    }

    qDebug() << "Success! Dynamic campus map engine loaded" << campusBuildings.size() << "buildings.";

    update();
    return true;
}

void Campus3DWidget::buildFootprintMatching()
{
    m_footprintIndexForBuilding.fill(-1, campusBuildings.size());
    QMap<QString, QVector<int>> nameToIndices;
    for (int i = 0; i < (int)m_buildingFootprints.size(); ++i) {
        nameToIndices[m_buildingFootprints[i].name].append(i);
    }

    for (int bi = 0; bi < campusBuildings.size(); ++bi) {
        const BuildingData& b = campusBuildings[bi];
        auto it = nameToIndices.find(b.name);
        if (it == nameToIndices.end()) continue;
        int bestIdx = -1;
        double bestDist = 1e18;
        for (int fi : it.value()) {
            const BuildingFootprint& fp = m_buildingFootprints[fi];

            double cx = 0.0, cy = 0.0;
            for (const QPointF& pt : fp.outer) { cx += pt.x(); cy += pt.y(); }
            cx /= fp.outer.size();
            cy /= fp.outer.size();

            double dx = (cx - b.longitude) * 111000.0;
            double dz = (cy - b.latitude) * 111000.0;
            double dist = std::sqrt(dx * dx + dz * dz);

            if (dist < bestDist) { bestDist = dist; bestIdx = fi; }
        }

        const double MAX_MATCH_DIST = 120.0;
        if (bestIdx != -1 && bestDist <= MAX_MATCH_DIST) {
            m_footprintIndexForBuilding[bi] = bestIdx;
        }
    }

    int matched = 0;
    for (int idx : m_footprintIndexForBuilding) if (idx >= 0) matched++;
    qDebug() << "建筑真实轮廓匹配成功:" << matched << "/" << campusBuildings.size();
}

void Campus3DWidget::setNavigationPath(const QVector<QPointF>& pathLonLat)
{
    m_pathLonLat = pathLonLat;

    activeNavigationPath.clear();
    for (const QPointF& p : pathLonLat) {
        QVector3D worldPos = map2DTo3D(p.x(), p.y(), 0.5f);
        activeNavigationPath.append(worldPos);
    }
    update();
}

QVector<Campus3DWidget::RoadNodeLonLat> Campus3DWidget::getRoadNodesLonLat() const {
    QVector<RoadNodeLonLat> result;
    for (int i = 0; i < (int)m_originalRoadNodes.size(); ++i) {
        const RoadNode& node = m_originalRoadNodes[i];
        result.append({i, node.x, node.z});
    }
    return result;
}

QVector<Campus3DWidget::RoadEdgeInfo> Campus3DWidget::getRoadEdgesInfo() const {
    QVector<RoadEdgeInfo> result;
    for (const RoadEdge& edge : m_roadEdges) {
        int u = edge.u;
        int v = edge.v;
        if (u < 0 || u >= (int)m_originalRoadNodes.size() || v < 0 || v >= (int)m_originalRoadNodes.size())
            continue;
        const RoadNode& nodeU = m_originalRoadNodes[u];
        const RoadNode& nodeV = m_originalRoadNodes[v];
        // 计算球面距离或平面近似
        double dx = (nodeV.x - nodeU.x) * 111000.0;
        double dz = (nodeV.z - nodeU.z) * 111000.0;
        double length = sqrt(dx*dx + dz*dz);
        result.append({u, v, length});
    }
    return result;
}
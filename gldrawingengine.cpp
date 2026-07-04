#include "gldrawingengine.h"
#include <cmath>
#include <algorithm>
void GLDrawingEngine::drawCube(QOpenGLFunctions* f, QVector3D center, float width, float length, float height, float r, float g, float b)
{
    float hW = width / 2.0f;
    float hL = length / 2.0f;
    glColor3f(r, g, b);
    glBegin(GL_QUADS);

    // FRONT
    glVertex3f(center.x() - hW, center.y(),          center.z() + hL);
    glVertex3f(center.x() + hW, center.y(),          center.z() + hL);
    glVertex3f(center.x() + hW, center.y() + height, center.z() + hL);
    glVertex3f(center.x() - hW, center.y() + height, center.z() + hL);
    // BACK
    glVertex3f(center.x() - hW, center.y(),          center.z() - hL);
    glVertex3f(center.x() - hW, center.y() + height, center.z() - hL);
    glVertex3f(center.x() + hW, center.y() + height, center.z() - hL);
    glVertex3f(center.x() + hW, center.y(),          center.z() - hL);
    // TOP
    glVertex3f(center.x() - hW, center.y() + height, center.z() - hL);
    glVertex3f(center.x() - hW, center.y() + height, center.z() + hL);
    glVertex3f(center.x() + hW, center.y() + height, center.z() + hL);
    glVertex3f(center.x() + hW, center.y() + height, center.z() - hL);
    // BOTTOM
    glVertex3f(center.x() - hW, center.y(),          center.z() - hL);
    glVertex3f(center.x() + hW, center.y(),          center.z() - hL);
    glVertex3f(center.x() + hW, center.y(),          center.z() + hL);
    glVertex3f(center.x() - hW, center.y(),          center.z() + hL);
    // LEFT
    glVertex3f(center.x() - hW, center.y(),          center.z() - hL);
    glVertex3f(center.x() - hW, center.y(),          center.z() + hL);
    glVertex3f(center.x() - hW, center.y() + height, center.z() + hL);
    glVertex3f(center.x() - hW, center.y() + height, center.z() - hL);
    // RIGHT
    glVertex3f(center.x() + hW, center.y(),          center.z() + hL);
    glVertex3f(center.x() + hW, center.y(),          center.z() - hL);
    glVertex3f(center.x() + hW, center.y() + height, center.z() - hL);
    glVertex3f(center.x() + hW, center.y() + height, center.z() + hL);

    glEnd();
}
void GLDrawingEngine::drawCourtyard(QOpenGLFunctions* f, QVector3D center,
                                    float width, float length, float height,
                                    float r, float g, float b,
                                    int openingDirection, float wallThickness)
{
    float hW = width / 2.0f;
    float hL = length / 2.0f;
    float wt = wallThickness;

    if (wt > width * 0.4f)  wt = width * 0.4f;
    if (wt > length * 0.4f) wt = length * 0.4f;

    switch (openingDirection) {
    case 0:
    case 2:
    {
        bool openNorth = (openingDirection == 0);
        float backZ = openNorth ? (center.z() + hL - wt / 2.0f)
                                : (center.z() - hL + wt / 2.0f);
        drawCube(f, QVector3D(center.x(), center.y(), backZ), width, wt, height, r, g, b);
        drawCube(f, QVector3D(center.x() - hW + wt / 2.0f, center.y(), center.z()),
                 wt, length, height, r, g, b);
        drawCube(f, QVector3D(center.x() + hW - wt / 2.0f, center.y(), center.z()),
                 wt, length, height, r, g, b);
        break;
    }
    case 1:
    case 3:
    {
        bool openEast = (openingDirection == 1);
        float backX = openEast ? (center.x() - hW + wt / 2.0f)
                               : (center.x() + hW - wt / 2.0f);

        drawCube(f, QVector3D(backX, center.y(), center.z()), wt, length, height, r, g, b);
        drawCube(f, QVector3D(center.x(), center.y(), center.z() - hL + wt / 2.0f),
                 width, wt, height, r, g, b);
        drawCube(f, QVector3D(center.x(), center.y(), center.z() + hL - wt / 2.0f),
                 width, wt, height, r, g, b);
        break;
    }
    default:
        drawCube(f, center, width, length, height, r, g, b);
        break;
    }
}
void GLDrawingEngine::drawRoute(QOpenGLFunctions* f, const QList<QVector3D>& pathPoints, float r, float g, float b)
{
    if(pathPoints.size() < 2) return;

    glLineWidth(4.0f);
    glColor3f(r,g,b);

    glBegin(GL_LINE_STRIP);
    for (const QVector3D& point : pathPoints){
        glVertex3f(point.x(), point.y() + 0.5f, point.z());
    }
    glEnd();
    glLineWidth(1.0f);
}

void GLDrawingEngine::drawRoadNetwork(QOpenGLFunctions* f, const std::vector<RoadNode>& nodes, const std::vector<RoadEdge>& edges, float r, float g, float b)
{
    if (edges.empty() || nodes.empty()) return;

    glColor3f(r, g, b);
    glLineWidth(2.5f);

    glBegin(GL_LINES);
    for (const auto& edge : edges) {
        if (edge.u < 0 || edge.u >= static_cast<int>(nodes.size()) ||
            edge.v < 0 || edge.v >= static_cast<int>(nodes.size())) {
            continue;
        }

        const RoadNode& nodeA = nodes[edge.u];
        const RoadNode& nodeB = nodes[edge.v];

        glVertex3f(nodeA.x, nodeA.y + 0.1f, nodeA.z);
        glVertex3f(nodeB.x, nodeB.y + 0.1f, nodeB.z);
    }
    glEnd();

    glLineWidth(1.0f);


}

void GLDrawingEngine::drawGroundPlane(QOpenGLFunctions* f, float halfSize, float y, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(-halfSize, y, -halfSize);
    glVertex3f(-halfSize, y,  halfSize);
    glVertex3f( halfSize, y,  halfSize);
    glVertex3f( halfSize, y, -halfSize);
    glEnd();
}

namespace {

double cross2D(const QVector3D& o, const QVector3D& a, const QVector3D& b)
{
    double ax = a.x() - o.x(), az = a.z() - o.z();
    double bx = b.x() - o.x(), bz = b.z() - o.z();
    return ax * bz - az * bx;
}

bool pointInTriangle(const QVector3D& p, const QVector3D& a, const QVector3D& b, const QVector3D& c)
{
    double d1 = cross2D(a, b, p);
    double d2 = cross2D(b, c, p);
    double d3 = cross2D(c, a, p);
    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return !(hasNeg && hasPos);
}

}

QList<QVector3D> GLDrawingEngine::triangulatePolygon(const QList<QVector3D>& inputPoints)
{
    QList<QVector3D> triangles;
    if (inputPoints.size() < 3) return triangles;

    QVector<QVector3D> pts = inputPoints.toVector();

    if (pts.size() > 1) {
        const QVector3D& first = pts.first();
        const QVector3D& last  = pts.last();
        if (qAbs(first.x() - last.x()) < 1e-9 && qAbs(first.z() - last.z()) < 1e-9) {
            pts.removeLast();
        }
    }
    if (pts.size() < 3) return triangles;

    double signedArea = 0.0;
    for (int i = 0; i < pts.size(); ++i) {
        const QVector3D& p1 = pts[i];
        const QVector3D& p2 = pts[(i + 1) % pts.size()];
        signedArea += (p1.x() * p2.z() - p2.x() * p1.z());
    }
    if (signedArea < 0) {
        std::reverse(pts.begin(), pts.end());
    }

    QVector<int> indices;
    indices.reserve(pts.size());
    for (int i = 0; i < pts.size(); ++i) indices.append(i);

    int guard = 0;
    const int maxGuard = pts.size() * pts.size() + 10;

    while (indices.size() > 3 && guard < maxGuard) {
        bool earFound = false;
        const int n = indices.size();

        for (int i = 0; i < n; ++i) {
            int iPrev = indices[(i - 1 + n) % n];
            int iCur  = indices[i];
            int iNext = indices[(i + 1) % n];

            const QVector3D& a = pts[iPrev];
            const QVector3D& b = pts[iCur];
            const QVector3D& c = pts[iNext];

            if (cross2D(a, b, c) <= 0.0) continue;

            bool anyInside = false;
            for (int k = 0; k < n; ++k) {
                int idx = indices[k];
                if (idx == iPrev || idx == iCur || idx == iNext) continue;
                if (pointInTriangle(pts[idx], a, b, c)) {
                    anyInside = true;
                    break;
                }
            }
            if (anyInside) continue;

            triangles.append(a);
            triangles.append(b);
            triangles.append(c);
            indices.remove(i);
            earFound = true;
            break;
        }

        if (!earFound) {
            break;
        }
        guard++;
    }

    if (indices.size() == 3) {
        triangles.append(pts[indices[0]]);
        triangles.append(pts[indices[1]]);
        triangles.append(pts[indices[2]]);
    } else if (indices.size() > 3) {
        for (int i = 1; i + 1 < indices.size(); ++i) {
            triangles.append(pts[indices[0]]);
            triangles.append(pts[indices[i]]);
            triangles.append(pts[indices[i + 1]]);
        }
    }

    return triangles;
}
namespace {
double signedArea2D_forHoles(const QVector<QVector3D>& pts)
{
    double area = 0.0;
    for (int i = 0; i < pts.size(); ++i) {
        const QVector3D& p1 = pts[i];
        const QVector3D& p2 = pts[(i + 1) % pts.size()];
        area += (p1.x() * p2.z() - p2.x() * p1.z());
    }
    return area;
}
}

QList<QVector3D> GLDrawingEngine::triangulatePolygonWithHoles(const QList<QVector3D>& outerIn,
                                                              const QList<QList<QVector3D>>& holesIn)
{
    if (holesIn.isEmpty()) {
        return triangulatePolygon(outerIn);
    }

    QVector<QVector3D> merged = outerIn.toVector();
    if (merged.size() > 1 &&
        qAbs(merged.first().x() - merged.last().x()) < 1e-9 &&
        qAbs(merged.first().z() - merged.last().z()) < 1e-9) {
        merged.removeLast();
    }
    if (signedArea2D_forHoles(merged) < 0) {
        std::reverse(merged.begin(), merged.end());
    }

    const float EPS = 0.0005f;

    for (const QList<QVector3D>& holeInList : holesIn) {
        if (holeInList.size() < 3) continue;

        QVector<QVector3D> hole = holeInList.toVector();
        if (hole.size() > 1 &&
            qAbs(hole.first().x() - hole.last().x()) < 1e-9 &&
            qAbs(hole.first().z() - hole.last().z()) < 1e-9) {
            hole.removeLast();
        }
        if (hole.size() < 3) continue;

        if (signedArea2D_forHoles(hole) > 0) {
            std::reverse(hole.begin(), hole.end());
        }

        int hIdx = 0;
        for (int i = 1; i < hole.size(); ++i) {
            if (hole[i].x() > hole[hIdx].x()) hIdx = i;
        }
        int oIdx = 0;
        double bestDist = 1e18;
        for (int i = 0; i < merged.size(); ++i) {
            double dx = merged[i].x() - hole[hIdx].x();
            double dz = merged[i].z() - hole[hIdx].z();
            double d = dx * dx + dz * dz;
            if (d < bestDist) { bestDist = d; oIdx = i; }
        }
        QVector<QVector3D> newMerged;
        for (int i = 0; i <= oIdx; ++i) newMerged.append(merged[i]);

        QVector3D bridgeStart = hole[hIdx];
        bridgeStart += QVector3D(EPS, 0, EPS);
        newMerged.append(bridgeStart);

        for (int i = 1; i < hole.size(); ++i) {
            newMerged.append(hole[(hIdx + i) % hole.size()]);
        }
        newMerged.append(hole[hIdx]);

        QVector3D bridgeEnd = merged[oIdx];
        bridgeEnd += QVector3D(-EPS, 0, -EPS);
        newMerged.append(bridgeEnd);

        for (int i = oIdx + 1; i < merged.size(); ++i) newMerged.append(merged[i]);

        merged = newMerged;
    }

    QList<QVector3D> mergedList;
    mergedList.reserve(merged.size());
    for (const QVector3D& p : merged) mergedList.append(p);

    return triangulatePolygon(mergedList);
}

void GLDrawingEngine::drawExtrudedFootprint(QOpenGLFunctions* f, const QList<QVector3D>& outer,
                                            const QList<QList<QVector3D>>& holes,
                                            float baseY, float height, float r, float g, float b)
{
    if (outer.size() < 3) return;
    float roofY = baseY + height;
    QList<QVector3D> roofTriangles = triangulatePolygonWithHoles(outer, holes);
    drawTriangleMesh(f, roofTriangles, roofY, r, g, b);

    glColor3f(r * 0.82f, g * 0.82f, b * 0.82f);
    glBegin(GL_QUADS);

    auto drawRingWalls = [&](const QList<QVector3D>& ring) {
        int n = ring.size();
        for (int i = 0; i < n; ++i) {
            const QVector3D& p1 = ring[i];
            const QVector3D& p2 = ring[(i + 1) % n];
            glVertex3f(p1.x(), baseY, p1.z());
            glVertex3f(p2.x(), baseY, p2.z());
            glVertex3f(p2.x(), roofY, p2.z());
            glVertex3f(p1.x(), roofY, p1.z());
        }
    };

    drawRingWalls(outer);
    for (const QList<QVector3D>& hole : holes) {
        drawRingWalls(hole);
    }

    glEnd();
}
void GLDrawingEngine::drawTriangleMesh(QOpenGLFunctions* f, const QList<QVector3D>& triangleVerts, float y, float r, float g, float b)
{
    if (triangleVerts.isEmpty()) return;
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLES);
    for (const QVector3D& p : triangleVerts) {
        glVertex3f(p.x(), y, p.z());
    }
    glEnd();
}

void GLDrawingEngine::drawFilledArea(QOpenGLFunctions* f, const QList<QVector3D>& points, float y, float r, float g, float b)
{
    QList<QVector3D> triangles = triangulatePolygon(points);
    drawTriangleMesh(f, triangles, y, r, g, b);
}

void GLDrawingEngine::drawRoadsSolid(QOpenGLFunctions* f, const std::vector<RoadNode>& nodes, const std::vector<RoadEdge>& edges,
                                     float roadWidth, float y, float r, float g, float b)
{
    if (edges.empty() || nodes.empty()) return;

    float halfW = roadWidth / 2.0f;
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    for (const auto& edge : edges) {
        if (edge.u < 0 || edge.u >= static_cast<int>(nodes.size()) ||
            edge.v < 0 || edge.v >= static_cast<int>(nodes.size())) {
            continue;
        }

        const RoadNode& a = nodes[edge.u];
        const RoadNode& b2 = nodes[edge.v];

        float dx = b2.x - a.x;
        float dz = b2.z - a.z;
        float len = std::sqrt(dx * dx + dz * dz);
        if (len < 0.001f) continue;
        float nx = -dz / len;
        float nz =  dx / len;

        glVertex3f(a.x  + nx * halfW, y, a.z  + nz * halfW);
        glVertex3f(a.x  - nx * halfW, y, a.z  - nz * halfW);
        glVertex3f(b2.x - nx * halfW, y, b2.z - nz * halfW);
        glVertex3f(b2.x + nx * halfW, y, b2.z + nz * halfW);
    }
    glEnd();
}
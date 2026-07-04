#ifndef GLDRAWINGENGINE_H
#define GLDRAWINGENGINE_H

#include <QVector3D>
#include <QOpenGLFunctions>
#include <QList>
#include "maploader.h"

class GLDrawingEngine
{
public:
    static void drawCube(QOpenGLFunctions* f, QVector3D center, float width, float length, float height, float r, float g, float b);
    static void drawRoute(QOpenGLFunctions* f, const QList<QVector3D>& pathPoints, float r, float g, float b);
    static void drawCourtyard(QOpenGLFunctions* f, QVector3D center, float width, float length, float height,
                              float r, float g, float b, int openingDirection, float wallThickness = 6.0f);
    static void drawRoadNetwork(QOpenGLFunctions* f, const std::vector<RoadNode>& nodes, const std::vector<RoadEdge>& edges, float r, float g, float b);
    static void drawGroundPlane(QOpenGLFunctions* f, float halfSize, float y, float r, float g, float b);
    static void drawFilledArea(QOpenGLFunctions* f, const QList<QVector3D>& points, float y, float r, float g, float b);
    static void drawRoadsSolid(QOpenGLFunctions* f, const std::vector<RoadNode>& nodes, const std::vector<RoadEdge>& edges,
                               float roadWidth, float y, float r, float g, float b);
    static QList<QVector3D> triangulatePolygon(const QList<QVector3D>& points);
    static void drawTriangleMesh(QOpenGLFunctions* f, const QList<QVector3D>& triangleVerts, float y, float r, float g, float b);
    static QList<QVector3D> triangulatePolygonWithHoles(const QList<QVector3D>& outer,
                                                        const QList<QList<QVector3D>>& holes);

    static void drawExtrudedFootprint(QOpenGLFunctions* f, const QList<QVector3D>& outer,
                                      const QList<QList<QVector3D>>& holes,
                                      float baseY, float height, float r, float g, float b);
};

#endif // GLDRAWINGENGINE_H
#ifndef BUILDING_H
#define BUILDING_H
#include <QString>
#include <QVector3D>
struct BuildingData {
    QString name;
    double longitude;
    double latitude;
    float width;
    float length;
    float height;
    float r, g, b;

    bool isCourtyard = false;
    int openingDirection = 2;
    float wallThickness = 6.0f;
};
#endif // BUILDING_H
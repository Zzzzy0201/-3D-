#ifndef MAPLOADER_H
#define MAPLOADER_H

#include <string>
#include <vector>
#include <QString>
#include <QVector>
#include <QPointF>

struct RoadNode {
    int id;
    float x;
    float y;
    float z;
};

struct RoadEdge {
    int u, v;
    std::string name;
};
struct BuildingFootprint {
    QString name;
    QVector<QPointF> outer;
    QVector<QVector<QPointF>> holes;
};


void loadRoadNetwork(const std::string& filename,
                     std::vector<RoadNode>& nodes,
                     std::vector<RoadEdge>& edges);

void loadBuildingFootprints(const std::string& filename, std::vector<BuildingFootprint>& footprints);

#endif // MAPLOADER_H
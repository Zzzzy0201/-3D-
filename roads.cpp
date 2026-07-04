#ifndef MAPLOADER_H
#define MAPLOADER_H

#include <string>
#include <vector>

// Structure for intersection points
struct RoadNode {
    int id;
    float x, y, z;
};

// Structure for the roads connecting the intersections
struct RoadEdge {
    int u, v;
    std::string name;
};

// Declaration of the function that will read the file
void loadRoadNetwork(const std::string& filename,
                     std::vector<RoadNode>& nodes,
                     std::vector<RoadEdge>& edges);

#endif // MAPLOADER_H
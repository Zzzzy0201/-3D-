#ifndef ROUTEPLANNER_H
#define ROUTEPLANNER_H

#include <vector>
#include <string>

// 偏离推荐结构体
struct DetourRecommendation {
    int poiId;
    std::string name;
    double detourTime;      // 偏离额外时间（分钟）
    double detourDistance;  // 偏离额外距离（米）
    double originalTime;    // 原路线时间
    double newTime;         // 加入后的新时间
    int stayTime;           // 停留时间
    std::vector<std::string> tags;
};

struct Road;

struct RoadNodeForPlanner {
    int id;
    double lon;
    double lat;
};
struct RoadEdgeForPlanner {
    int from;
    int to;
    double distance;
};

class RoutePlanner {
public:
    RoutePlanner();
    ~RoutePlanner();

    bool loadFromCSV(const std::string& filename);
    int findPOIByName(const std::string& name) const;
    std::string getPOIName(int id) const;
    bool getBuildingLonLat(int id, double &lon, double &lat) const;
    std::vector<int> planRoute(int startId, const std::vector<int>& destIds,
                               double& totalTime, double& totalDistance) const;
    int getPOICount() const;
    const std::vector<Road>& getRoads() const;

    // 沿途推荐
    std::vector<DetourRecommendation> searchByTagAndSortByDetour(
        const std::vector<int>& currentPath,
        const std::string& targetTag) const;

    DetourRecommendation calculateDetour(const std::vector<int>& path, int poiId) const;
    std::vector<std::vector<double>> getPathTurningPoints(
        const std::string& start_poi_name,
        const std::string& end_poi_name) const;

    void setRoadNetwork(const std::vector<RoadNodeForPlanner>& nodes,
                        const std::vector<RoadEdgeForPlanner>& edges);


private:
    class Impl;
    Impl* m_impl;
};

#endif
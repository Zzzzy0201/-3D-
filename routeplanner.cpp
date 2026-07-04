#include "routeplanner.h"
#include <vector>
#include <string>
#include <cmath>
#include <queue>
#include <algorithm>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QFileInfo>

using namespace std;

const double PI = 3.14159265358979323846;
const double INF_DOUBLE = 1e12;
//道路网络相关结构
struct RoadPoint {
    double lon;
    double lat;
    RoadPoint(double lng = 0, double la = 0) : lon(lng), lat(la) {}

    bool operator<(const RoadPoint& other) const {
        if (lon != other.lon) return lon < other.lon;
        return lat < other.lat;
    }

    double distanceTo(const RoadPoint& other) const {
        const double R = 6371000;
        double lat1 = lat * PI / 180.0;
        double lat2 = other.lat * PI / 180.0;
        double deltaLat = (other.lat - lat) * PI / 180.0;
        double deltaLon = (other.lon - lon) * PI / 180.0;
        double a = sin(deltaLat / 2) * sin(deltaLat / 2) +
                   cos(lat1) * cos(lat2) * sin(deltaLon / 2) * sin(deltaLon / 2);
        double c = 2 * atan2(sqrt(a), sqrt(1 - a));
        return R * c;
    }
};

struct Road {
    string id;
    string type;
    string name;
    string highway;
    int point_count;
    vector<RoadPoint> points;
};

// 道路网络图结构
struct GraphNode {
    int id;
    RoadPoint point;
    vector<pair<int, double>> neighbors; // neighbor id, distance
};

class RoadNetworkPathPlanner {
private:
    vector<Road> roads;
    vector<GraphNode> graph;
    map<RoadPoint, int> point_to_id;
    map<int, RoadPoint> id_to_point;

    double calculateAngle(const RoadPoint& p1, const RoadPoint& p2, const RoadPoint& p3) const{
        double v1x = p2.lon - p1.lon;
        double v1y = p2.lat - p1.lat;
        double v2x = p3.lon - p2.lon;
        double v2y = p3.lat - p2.lat;
        double dot = v1x * v2x + v1y * v2y;
        double norm1 = sqrt(v1x * v1x + v1y * v1y);
        double norm2 = sqrt(v2x * v2x + v2y * v2y);
        if (norm1 < 1e-8 || norm2 < 1e-8) return 0;
        double cos_theta = dot / (norm1 * norm2);
        cos_theta = max(-1.0, min(1.0, cos_theta));
        double theta_rad = acos(cos_theta);
        return theta_rad * 180.0 / PI;
    }

    vector<RoadPoint> extractTurningPoints(const vector<int>& path) const{
        vector<RoadPoint> all_points;
        for (int id : path) {
            all_points.push_back(id_to_point.at(id));
        }
        return all_points;
    }

    // Dijkstra 最短路径
    vector<int> findShortestPath(int start_id, int end_id) const{
        int n = graph.size();
        vector<double> dist(n, INF_DOUBLE);
        vector<int> prev(n, -1);
        vector<bool> visited(n, false);
        using QueuePair = pair<double, int>;
        priority_queue<QueuePair, vector<QueuePair>, greater<QueuePair>> pq;
        dist[start_id] = 0;
        pq.push({0, start_id});
        while (!pq.empty()) {
            int u = pq.top().second;
            pq.pop();
            if (visited[u]) continue;
            visited[u] = true;
            if (u == end_id) break;
            for (const auto& nb : graph[u].neighbors) {
                int v = nb.first;
                double w = nb.second;
                if (!visited[v] && dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
        vector<int> path;
        if (prev[end_id] == -1 && start_id != end_id) return path;
        int cur = end_id;
        while (cur != -1) {
            path.push_back(cur);
            cur = prev[cur];
        }
        reverse(path.begin(), path.end());
        return path;
    }

    // 将一条道路添加到图中（建立节点和边）
    void addRoadToGraph(const Road& road) {
        if (road.points.size() < 2) return;
        for (size_t i = 0; i < road.points.size(); ++i) {
            const RoadPoint& p = road.points[i];
            if (point_to_id.find(p) == point_to_id.end()) {
                int new_id = graph.size();
                point_to_id[p] = new_id;
                id_to_point[new_id] = p;
                GraphNode node;
                node.id = new_id;
                node.point = p;
                graph.push_back(node);
            }
        }
        for (size_t i = 0; i < road.points.size() - 1; ++i) {
            int id1 = point_to_id[road.points[i]];
            int id2 = point_to_id[road.points[i+1]];
            double dist = road.points[i].distanceTo(road.points[i+1]);
            graph[id1].neighbors.push_back({id2, dist});
            graph[id2].neighbors.push_back({id1, dist});
        }
    }

    void buildGraph() {
        graph.clear();
        point_to_id.clear();
        id_to_point.clear();
        for (const auto& road : roads) {
            addRoadToGraph(road);
        }
        qDebug() << "[DEBUG] Built graph with" << graph.size() << "nodes";
    }

    // 找到最近的 k 个节点
    std::vector<int> findNearestNodes(double lon, double lat, int k) const {
        RoadPoint target(lon, lat);
        std::vector<std::pair<double, int>> distances;
        for (const auto& node : graph) {
            double d = target.distanceTo(node.point);
            distances.push_back({d, node.id});
        }
        std::sort(distances.begin(), distances.end());
        std::vector<int> result;
        for (int i = 0; i < std::min(k, (int)distances.size()); ++i)
            result.push_back(distances[i].second);
        return result;
    }
    // 找到最近的一个节点
    int findNearestNode(double lon, double lat) const {
        auto nodes = findNearestNodes(lon, lat, 1);
        return nodes.empty() ? -1 : nodes[0];
    }
    std::vector<int> findNearestNodesWithinDistance(double lon, double lat, int k, double maxDistMeters) const {
        RoadPoint target(lon, lat);
        std::vector<std::pair<double, int>> distances;
        for (const auto& node : graph) {
            double d = target.distanceTo(node.point);
            if (d <= maxDistMeters) {
                distances.push_back({d, node.id});
            }
        }
        std::sort(distances.begin(), distances.end());
        std::vector<int> result;
        for (int i = 0; i < std::min(k, (int)distances.size()); ++i) {
            result.push_back(distances[i].second);
        }
        return result;
    }

public:
    void init(const vector<Road>& road_list) {
        roads = road_list;
        buildGraph();
    }
    vector<vector<double>> getPathTurningPoints(const string& start_poi_name, const string& end_poi_name, const RoutePlanner& poi_planner)
    {
        vector<vector<double>> result;
        int start_id = poi_planner.findPOIByName(start_poi_name);
        int end_id   = poi_planner.findPOIByName(end_poi_name);
        if (start_id == -1 || end_id == -1) return result;

        double lon1, lat1, lon2, lat2;
        if (!poi_planner.getBuildingLonLat(start_id, lon1, lat1) ||
            !poi_planner.getBuildingLonLat(end_id, lon2, lat2))
            return result;

        int start_node = findNearestNode(lon1, lat1);
        int end_node   = findNearestNode(lon2, lat2);
        if (start_node == -1 || end_node == -1) return result;

        vector<int> path = findShortestPath(start_node, end_node);
        if (path.empty()) return result;

        vector<RoadPoint> turning_points = extractTurningPoints(path);
        for (const auto& pt : turning_points) {
            result.push_back({pt.lon, pt.lat});
        }
        return result;
    }

    vector<vector<double>> getPathTurningPoints(double lon1, double lat1, double lon2, double lat2) const {
        vector<vector<double>> result;

        if (graph.empty()) {
            result.push_back({lon1, lat1});
            result.push_back({lon2, lat2});
            return result;
        }
        auto start_candidates = findNearestNodesWithinDistance(lon1, lat1, 10, 200.0);
        auto end_candidates   = findNearestNodesWithinDistance(lon2, lat2, 10, 200.0);

        if (start_candidates.empty() || end_candidates.empty()) {
            // 如果范围内没有节点，回退到最近的 5 个节点（不限距离）
            start_candidates = findNearestNodes(lon1, lat1, 5);
            end_candidates   = findNearestNodes(lon2, lat2, 5);
            if (start_candidates.empty() || end_candidates.empty()) {
                result.push_back({lon1, lat1});
                result.push_back({lon2, lat2});
                return result;
            }
        }

        // 尝试所有组合，找到第一条有效路径
        vector<int> best_path;
        for (int s : start_candidates) {
            for (int e : end_candidates) {
                vector<int> path = findShortestPath(s, e);
                if (!path.empty()) {
                    best_path = path;
                    break;
                }
            }
            if (!best_path.empty()) break;
        }

        if (best_path.empty()) {
            int s = findNearestNode(lon1, lat1);
            int e = findNearestNode(lon2, lat2);
            if (s != -1 && e != -1) {
                vector<int> path = findShortestPath(s, e);
                if (!path.empty()) best_path = path;
            }
            if (best_path.empty()) {
                result.push_back({lon1, lat1});
                result.push_back({lon2, lat2});
                return result;
            }
        }

        // 提取拐点
        vector<RoadPoint> turning_points = extractTurningPoints(best_path);
        for (const auto& pt : turning_points) {
            result.push_back({pt.lon, pt.lat});
        }
        return result;
    }

    void loadFromExternal(const std::vector<RoadNodeForPlanner>& nodes,
                          const std::vector<RoadEdgeForPlanner>& edges) {
        graph.clear();
        point_to_id.clear();
        id_to_point.clear();

        int maxId = -1;
        for (const auto& nd : nodes) {
            if (nd.id > maxId) maxId = nd.id;
        }
        graph.resize(maxId + 1);
        for (const auto& nd : nodes) {
            GraphNode node;
            node.id = nd.id;
            node.point = RoadPoint(nd.lon, nd.lat);
            graph[nd.id] = node;
            point_to_id[node.point] = nd.id;
            id_to_point[nd.id] = node.point;
        }

        for (const auto& ed : edges) {
            if (ed.from >= 0 && ed.from < (int)graph.size() &&
                ed.to   >= 0 && ed.to   < (int)graph.size()) {
                graph[ed.from].neighbors.push_back({ed.to, ed.distance});
                graph[ed.to].neighbors.push_back({ed.from, ed.distance});
            }
        }
        qDebug() << "[DEBUG] loadFromExternal: graph size=" << graph.size()
                 << "edges added=" << (graph.empty() ? 0 : [&](){ int c=0; for(auto& g:graph) c+=g.neighbors.size(); return c/2; }());
    }
};

const double WALKING_SPEED = 1.4;

struct Point {
    double longitude;
    double latitude;
    Point(double lon = 0, double lat = 0) : longitude(lon), latitude(lat) {}
    double distanceTo(const Point& other) const {
        const double R = 6371000;
        double lat1 = latitude * PI / 180.0;
        double lat2 = other.latitude * PI / 180.0;
        double deltaLat = (other.latitude - latitude) * PI / 180.0;
        double deltaLon = (other.longitude - longitude) * PI / 180.0;
        double a = sin(deltaLat / 2) * sin(deltaLat / 2) +
                   cos(lat1) * cos(lat2) * sin(deltaLon / 2) * sin(deltaLon / 2);
        double c = 2 * atan2(sqrt(a), sqrt(1 - a));
        return R * c;
    }
};

struct POI {
    int id;
    string name;
    Point location;
    int stayTime;
    vector<string> tags;
};

class MapData {
public:
    vector<POI> pois;
    vector<vector<double>> distMat;
    vector<vector<double>> timeMat;

    bool loadFromCSV(const string& filename) {
        QFile file(QString::fromStdString(filename));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return false;
        }
        QTextStream in(&file);
        QString line;
        // 跳过标题行
        if (in.atEnd()) return false;
        in.readLine();
        while (!in.atEnd()) {
            line = in.readLine();
            if (line.isEmpty()) continue;
            QStringList fields = line.split(',');
            if (fields.size() < 5) continue;
            POI poi;
            poi.name = fields[0].toStdString();
            poi.location.longitude = fields[1].toDouble();
            poi.location.latitude = fields[2].toDouble();
            poi.stayTime = fields[3].toInt();
            QStringList tagList = fields[4].split(';');
            for (const QString& tag : tagList) {
                poi.tags.push_back(tag.trimmed().toStdString());
            }
            poi.id = pois.size();
            pois.push_back(poi);
        }
        file.close();
        buildCompleteGraph();
        return !pois.empty();
    }

    void buildCompleteGraph() {
        int n = pois.size();
        if (n == 0) return;
        distMat.assign(n, vector<double>(n, INF_DOUBLE));
        timeMat.assign(n, vector<double>(n, INF_DOUBLE));
        for (int i = 0; i < n; ++i) distMat[i][i] = timeMat[i][i] = 0;
        for (int i = 0; i < n; ++i) {
            for (int j = i+1; j < n; ++j) {
                double d = pois[i].location.distanceTo(pois[j].location);
                double t = d / WALKING_SPEED / 60.0; // 分钟
                distMat[i][j] = distMat[j][i] = d;
                timeMat[i][j] = timeMat[j][i] = t;
            }
        }
    }

    double getDistance(int from, int to) const { return distMat[from][to]; }
    double getTime(int from, int to) const { return timeMat[from][to]; }
    const POI& getPOI(int id) const { return pois[id]; }
    int getPOICount() const { return pois.size(); }
};

class MultiDestinationPlanner {
    const MapData& map;
public:
    MultiDestinationPlanner(const MapData& m) : map(m) {}

    vector<int> optimizeOrder(int startId, const vector<int>& destIds) {
        int n = destIds.size();
        if (n == 0) return {};
        if (n == 1) return {destIds[0]};
        int dpSize = 1 << n;
        vector<vector<double>> dp(dpSize, vector<double>(n, INF_DOUBLE));
        vector<vector<int>> prev(dpSize, vector<int>(n, -1));
        for (int i = 0; i < n; ++i) {
            double t = map.getTime(startId, destIds[i]);
            if (t < INF_DOUBLE) dp[1<<i][i] = t;
        }
        for (int mask = 1; mask < dpSize; ++mask) {
            for (int last = 0; last < n; ++last) {
                if (!(mask & (1<<last))) continue;
                if (dp[mask][last] >= INF_DOUBLE) continue;
                for (int next = 0; next < n; ++next) {
                    if (mask & (1<<next)) continue;
                    double t = map.getTime(destIds[last], destIds[next]);
                    if (t >= INF_DOUBLE) continue;
                    int newMask = mask | (1<<next);
                    double newTime = dp[mask][last] + t;
                    if (newTime < dp[newMask][next]) {
                        dp[newMask][next] = newTime;
                        prev[newMask][next] = last;
                    }
                }
            }
        }
        int last = -1;
        double best = INF_DOUBLE;
        for (int i = 0; i < n; ++i) {
            if (dp[dpSize-1][i] < best) {
                best = dp[dpSize-1][i];
                last = i;
            }
        }
        if (last == -1) return {};
        vector<int> order;
        int mask = dpSize-1;
        while (last != -1) {
            order.push_back(destIds[last]);
            int nxt = prev[mask][last];
            mask &= ~(1<<last);
            last = nxt;
        }
        reverse(order.begin(), order.end());
        return order;
    }
};

class RoutePlanner::Impl {
public:
    MapData mapData;
    vector<Road> roads;   // 存储所有道路
    RoadNetworkPathPlanner roadPathPlanner;
    MultiDestinationPlanner* planner = nullptr;
    void setRoadNetwork(const std::vector<RoadNodeForPlanner>& nodes,
                        const std::vector<RoadEdgeForPlanner>& edges) {
        roadPathPlanner.loadFromExternal(nodes, edges);
    }

    ~Impl() { delete planner; }

    bool load(const string& fn, const string& road_fn = "") {
        if (!mapData.loadFromCSV(fn)) return false;
        planner = new MultiDestinationPlanner(mapData);
        if (!road_fn.empty()) {
            loadRoads(road_fn);
            roadPathPlanner.init(roads);  // 使用文件加载的道路数据
        }
        return true;
    }

    const vector<Road>& getRoads() const { return roads; }

    std::vector<std::vector<double>> getPathTurningPoints(
        const std::string& start_poi_name,
        const std::string& end_poi_name) const
    {
        int start_id = find(start_poi_name);
        int end_id   = find(end_poi_name);
        if (start_id == -1 || end_id == -1) return {};

        double lon1, lat1, lon2, lat2;
        if (!getBuildingLonLat(start_id, lon1, lat1) ||
            !getBuildingLonLat(end_id, lon2, lat2))
            return {};

        return roadPathPlanner.getPathTurningPoints(lon1, lat1, lon2, lat2);
    }

    bool loadRoads(const string& filename) {
        QFile file(QString::fromStdString(filename));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "[DEBUG] Failed to open road file:" << QString::fromStdString(filename);
            return false;
        }

        QTextStream in(&file);
        if (in.atEnd()) return false;
        QString header = in.readLine(); // 跳过标题行

        int lineNum = 1;
        while (!in.atEnd()) {
            QString line = in.readLine();
            lineNum++;
            if (line.trimmed().isEmpty()) continue;

            QStringList fields = line.split(',');
            if (fields.size() < 6) {
                qDebug() << "[DEBUG] Skipping line" << lineNum << ": insufficient fields" << fields.size();
                continue;
            }

            Road road;
            road.id = fields[0].toStdString();
            road.type = fields[1].toStdString();
            road.name = fields[2].toStdString();
            road.highway = fields[3].toStdString();
            road.point_count = fields[4].toInt();

            QString coordStr = fields[5];
            if (coordStr.startsWith('"') && coordStr.endsWith('"')) {
                coordStr = coordStr.mid(1, coordStr.length() - 2);
            }
            // 按分号分割
            QStringList pointsStr = coordStr.split(';');
            for (const QString& ptStr : pointsStr) {
                QStringList lonlat = ptStr.split(',');
                if (lonlat.size() >= 2) {
                    double lon = lonlat[0].toDouble();
                    double lat = lonlat[1].toDouble();
                    road.points.push_back(RoadPoint(lon, lat));
                }
            }
            if (road.highway != "elevator" && road.highway != "steps") {
                roads.push_back(road);
            }
        }

        file.close();
        qDebug() << "[DEBUG] Loaded" << roads.size() << "roads from" << QString::fromStdString(filename);
        return !roads.empty();
    }

    int find(const string& name) const {
        for (int i = 0; i < mapData.getPOICount(); ++i)
            if (mapData.getPOI(i).name == name) return i;
        return -1;
    }

    string getName(int id) const { return mapData.getPOI(id).name; }

    vector<int> plan(int startId, const vector<int>& destIds, double& totalTime, double& totalDistance) const {
        vector<int> ordered = planner->optimizeOrder(startId, destIds);
        vector<int> fullPath;
        fullPath.push_back(startId);
        fullPath.insert(fullPath.end(), ordered.begin(), ordered.end());

        totalDistance = 0;
        totalTime = 0;
        for (size_t i = 0; i+1 < fullPath.size(); ++i) {
            double d = mapData.getDistance(fullPath[i], fullPath[i+1]);
            double t = mapData.getTime(fullPath[i], fullPath[i+1]);
            totalDistance += d;
            totalTime += t;
        }
        for (size_t i = 1; i < fullPath.size(); ++i) {
            totalTime += mapData.getPOI(fullPath[i]).stayTime / 60.0;
        }
        return ordered;
    }

    int getCount() const { return mapData.getPOICount(); }

    bool getBuildingLonLat(int id, double &lon, double &lat) const {
        if (id < 0 || id >= mapData.getPOICount()) return false;
        const POI& poi = mapData.getPOI(id);
        lon = poi.location.longitude;
        lat = poi.location.latitude;
        return true;
    }

    DetourRecommendation calculateDetour(const std::vector<int>& path, int poiId) const {
        DetourRecommendation result;
        result.poiId = poiId;
        result.name = mapData.getPOI(poiId).name;
        result.stayTime = mapData.getPOI(poiId).stayTime;
        result.tags = mapData.getPOI(poiId).tags;
        result.detourTime = INF_DOUBLE;
        result.detourDistance = INF_DOUBLE;

        double originalTotal = 0;
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            originalTotal += mapData.getTime(path[i], path[i + 1]);
        }
        result.originalTime = originalTotal;

        double bestNewTotal = INF_DOUBLE;
        int bestPos = -1;

        for (size_t pos = 0; pos <= path.size(); ++pos) {
            double newTotal = 0;
            for (size_t i = 0; i + 1 < pos; ++i) {
                newTotal += mapData.getTime(path[i], path[i + 1]);
            }
            if (pos == 0) {
                newTotal += mapData.getTime(poiId, path[0]);
            } else if (pos == path.size()) {
                newTotal += mapData.getTime(path[pos - 1], poiId);
            } else {
                newTotal += mapData.getTime(path[pos - 1], poiId);
                newTotal += mapData.getTime(poiId, path[pos]);
            }
            for (size_t i = pos; i + 1 < path.size(); ++i) {
                newTotal += mapData.getTime(path[i], path[i + 1]);
            }
            if (newTotal < bestNewTotal) {
                bestNewTotal = newTotal;
                bestPos = pos;
            }
        }

        result.newTime = bestNewTotal;
        result.detourTime = bestNewTotal - originalTotal;

        if (bestPos == 0) {
            result.detourDistance = mapData.getDistance(poiId, path[0]);
        } else if (bestPos == (int)path.size()) {
            result.detourDistance = mapData.getDistance(path[bestPos - 1], poiId);
        } else {
            double oldDist = mapData.getDistance(path[bestPos - 1], path[bestPos]);
            double newDist = mapData.getDistance(path[bestPos - 1], poiId) +
                             mapData.getDistance(poiId, path[bestPos]);
            result.detourDistance = newDist - oldDist;
        }
        return result;
    }

    std::vector<DetourRecommendation> searchByTagAndSortByDetour(
        const std::vector<int>& currentPath,
        const std::string& targetTag) const
    {
        std::vector<DetourRecommendation> results;
        int n = mapData.getPOICount();
        for (int i = 0; i < n; ++i) {
            const POI& poi = mapData.getPOI(i);
            bool hasTag = false;
            for (const auto& tag : poi.tags) {
                if (tag == targetTag) {
                    hasTag = true;
                    break;
                }
            }
            if (!hasTag) continue;
            if (std::find(currentPath.begin(), currentPath.end(), i) != currentPath.end()) continue;
            DetourRecommendation detour = calculateDetour(currentPath, i);
            if (detour.detourTime < INF_DOUBLE / 2) {
                results.push_back(detour);
            }
        }
        std::sort(results.begin(), results.end(),
                  [](const DetourRecommendation& a, const DetourRecommendation& b) {
                      return a.detourTime < b.detourTime;
                  });
        return results;
    }

};
RoutePlanner::RoutePlanner() : m_impl(new Impl) {}
RoutePlanner::~RoutePlanner() { delete m_impl; }
bool RoutePlanner::loadFromCSV(const string& filename) { return m_impl->load(filename); }
int RoutePlanner::findPOIByName(const string& name) const { return m_impl->find(name); }
string RoutePlanner::getPOIName(int id) const { return m_impl->getName(id); }
vector<int> RoutePlanner::planRoute(int startId, const vector<int>& destIds, double& totalTime, double& totalDistance) const {
    return m_impl->plan(startId, destIds, totalTime, totalDistance);
}
int RoutePlanner::getPOICount() const { return m_impl->getCount(); }

bool RoutePlanner::getBuildingLonLat(int id, double &lon, double &lat) const {
    return m_impl->getBuildingLonLat(id, lon, lat);
}

std::vector<DetourRecommendation> RoutePlanner::searchByTagAndSortByDetour(
    const std::vector<int>& currentPath,
    const std::string& targetTag) const
{
    return m_impl->searchByTagAndSortByDetour(currentPath, targetTag);
}

DetourRecommendation RoutePlanner::calculateDetour(const std::vector<int>& path, int poiId) const
{
    return m_impl->calculateDetour(path, poiId);
}

const vector<Road>& RoutePlanner::getRoads() const {
    return m_impl->getRoads();
}

std::vector<std::vector<double>> RoutePlanner::getPathTurningPoints(
    const std::string& start_poi_name,
    const std::string& end_poi_name) const
{
    return m_impl->getPathTurningPoints(start_poi_name, end_poi_name);
}

void RoutePlanner::setRoadNetwork(const std::vector<RoadNodeForPlanner>& nodes,
                                  const std::vector<RoadEdgeForPlanner>& edges) {
    m_impl->setRoadNetwork(nodes, edges);
}
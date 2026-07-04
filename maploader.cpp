#include "maploader.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMap>

void loadRoadNetwork(const std::string& filename,
                     std::vector<RoadNode>& nodes,
                     std::vector<RoadEdge>& edges) {

    QFile file(QString::fromStdString(filename));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: Could not open road file at" << filename.c_str();
        return;
    }

    QTextStream in(&file);
    if (in.atEnd()) return;
    in.readLine();
    QMap<QString, int> coordToNodeId;
    int globalNodeId = 0;

    auto makeKey = [](double lon, double lat) -> QString {
        return QString("%1_%2").arg(lon, 0, 'f', 7).arg(lat, 0, 'f', 7);
    };

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty()) continue;
        if (line.endsWith('\r')) line.chop(1);
        if (line.isEmpty()) continue;
        int commaCount = 0;
        int lastPos = 0;
        int pos = 0;
        QString fields[6];
        while (commaCount < 5 && pos < line.length()) {
            if (line[pos] == ',') {
                fields[commaCount] = line.mid(lastPos, pos - lastPos);
                lastPos = pos + 1;
                commaCount++;
            }
            pos++;
        }
        if (commaCount == 5) {
            fields[5] = line.mid(lastPos);
        } else {
            continue;
        }

        QString id = fields[0].trimmed();
        QString type = fields[1].trimmed();
        QString name = fields[2].trimmed();
        QString highway = fields[3].trimmed();
        QString count = fields[4].trimmed();
        QString coords_str = fields[5];
        if (coords_str.startsWith('"')) coords_str = coords_str.mid(1);
        if (coords_str.endsWith('"')) coords_str.chop(1);
        if (coords_str.isEmpty()) continue;

        // 分割坐标对
        QStringList coordPairs = coords_str.split(';', Qt::SkipEmptyParts);
        int previousNodeId = -1;

        for (const QString& pair : coordPairs) {
            QStringList lonLat = pair.split(',');
            if (lonLat.size() != 2) continue;

            bool ok1, ok2;
            double lon = lonLat[0].trimmed().toDouble(&ok1);
            double lat = lonLat[1].trimmed().toDouble(&ok2);
            if (!ok1 || !ok2) continue;

            QString key = makeKey(lon, lat);
            int currentNodeId;
            auto it = coordToNodeId.constFind(key);
            if (it != coordToNodeId.constEnd()) {
                currentNodeId = it.value();
            } else {
                currentNodeId = globalNodeId++;
                coordToNodeId.insert(key, currentNodeId);

                RoadNode node;
                node.id = currentNodeId;
                node.x = lon;
                node.y = 0.0f;
                node.z = lat;
                nodes.push_back(node);
            }

            if (previousNodeId != -1 && previousNodeId != currentNodeId) {
                RoadEdge edge;
                edge.u = previousNodeId;
                edge.v = currentNodeId;
                edge.name = (name.isEmpty() ? highway : name).toStdString();
                edges.push_back(edge);
            }

            previousNodeId = currentNodeId;
        }
    }

    file.close();
    qDebug() << "Successfully parsed" << nodes.size() << "nodes and"
             << edges.size() << "edges from road file (with intersection dedup).";
}
void loadBuildingFootprints(const std::string& filename, std::vector<BuildingFootprint>& footprints)
{
    QFile file(QString::fromStdString(filename));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: Could not open building footprint file at" << filename.c_str();
        return;
    }

    QTextStream in(&file);
    if (in.atEnd()) return;
    in.readLine();

    auto parseRing = [](const QString& ringStr) -> QVector<QPointF> {
        QVector<QPointF> ring;
        QStringList pairs = ringStr.split(';', Qt::SkipEmptyParts);
        for (const QString& pair : pairs) {
            QStringList lonLat = pair.split(',');
            if (lonLat.size() != 2) continue;
            bool ok1, ok2;
            double lon = lonLat[0].trimmed().toDouble(&ok1);
            double lat = lonLat[1].trimmed().toDouble(&ok2);
            if (ok1 && ok2) ring.append(QPointF(lon, lat));
        }
        return ring;
    };

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty()) continue;
        if (line.endsWith('\r')) line.chop(1);
        if (line.isEmpty() || line.startsWith('#')) continue;

        QStringList fields = line.split('|');
        if (fields.size() < 2) continue;

        BuildingFootprint fp;
        fp.name = fields[0].trimmed();
        fp.outer = parseRing(fields[1]);
        if (fp.outer.size() < 3) continue;

        for (int i = 2; i < fields.size(); ++i) {
            QVector<QPointF> hole = parseRing(fields[i]);
            if (hole.size() >= 3) {
                fp.holes.append(hole);
            }
        }

        footprints.push_back(fp);
    }

    file.close();
    qDebug() << "Successfully loaded" << footprints.size() << "building footprints from" << QString::fromStdString(filename);
}
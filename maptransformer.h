#ifndef MAPTRANSFORMER_H
#define MAPTRANSFORMER_H

#include <QPointF>
#include <QVector>
#include <QTransform>

class MapTransformer
{
public:
    MapTransformer();

    void addReferencePoint(double lon, double lat, double px, double py);
    bool compute();
    QPointF lonLatToPx(double lon, double lat) const;
    void pxToLonLat(double px, double py, double &lon, double &lat) const;

private:
    QVector<QPointF> srcPoints;   // 经纬度点 (lon, lat)
    QVector<QPointF> dstPoints;   // 像素点 (px, py)
    QTransform transform;         // 正向变换
    QTransform invTransform;      // 反向变换
    bool valid;
};

#endif // MAPTRANSFORMER_H
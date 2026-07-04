#include "maptransformer.h"
#include <QtMath>

MapTransformer::MapTransformer() : valid(false) {}

void MapTransformer::addReferencePoint(double lon, double lat, double px, double py)
{
    srcPoints.append(QPointF(lon, lat));
    dstPoints.append(QPointF(px, py));
}

bool MapTransformer::compute()
{
    int n = srcPoints.size();
    QVector<QVector<double>> A(2*n, QVector<double>(6, 0.0));
    QVector<double> B(2*n, 0.0);

    for (int i = 0; i < n; ++i) {
        double lon = srcPoints[i].x();
        double lat = srcPoints[i].y();
        double px = dstPoints[i].x();
        double py = dstPoints[i].y();

        A[2*i][0] = lon;
        A[2*i][1] = lat;
        A[2*i][2] = 1.0;
        B[2*i] = px;

        A[2*i+1][3] = lon;
        A[2*i+1][4] = lat;
        A[2*i+1][5] = 1.0;
        B[2*i+1] = py;
    }

    // 最小二乘求解
    QVector<QVector<double>> ATA(6, QVector<double>(6, 0.0));
    QVector<double> ATB(6, 0.0);
    for (int i = 0; i < 2*n; ++i) {
        for (int j = 0; j < 6; ++j) {
            for (int k = 0; k < 6; ++k) {
                ATA[j][k] += A[i][j] * A[i][k];
            }
            ATB[j] += A[i][j] * B[i];
        }
    }
    QVector<QVector<double>> mat(6, QVector<double>(7, 0.0));
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) {
            mat[i][j] = ATA[i][j];
        }
        mat[i][6] = ATB[i];
    }

    for (int i = 0; i < 6; ++i) {
        int maxRow = i;
        for (int r = i+1; r < 6; ++r) {
            if (qAbs(mat[r][i]) > qAbs(mat[maxRow][i]))
                maxRow = r;
        }
        if (maxRow != i) {
            mat[i].swap(mat[maxRow]);
        }

        double pivot = mat[i][i];
        for (int j = i; j <= 6; ++j)
            mat[i][j] /= pivot;
        for (int r = 0; r < 6; ++r) {
            if (r != i && qAbs(mat[r][i]) > 1e-12) {
                double factor = mat[r][i];
                for (int j = i; j <= 6; ++j)
                    mat[r][j] -= factor * mat[i][j];
            }
        }
    }
    double a = mat[0][6];
    double b = mat[1][6];
    double c = mat[2][6];
    double d = mat[3][6];
    double e = mat[4][6];
    double f = mat[5][6];

    transform = QTransform(a, d, 0,
                           b, e, 0,
                           c, f, 1);
    bool invertible;
    invTransform = transform.inverted(&invertible);
    valid = true;
    return true;
}

QPointF MapTransformer::lonLatToPx(double lon, double lat) const
{
    if (!valid) return QPointF(-1, -1);
    return transform.map(QPointF(lon, lat));
}

void MapTransformer::pxToLonLat(double px, double py, double &lon, double &lat) const
{
    if (!valid) return;
    QPointF p = invTransform.map(QPointF(px, py));
    lon = p.x();
    lat = p.y();
}
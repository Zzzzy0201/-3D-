#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QPoint>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLFunctions>

class CameraManager
{
public:
    CameraManager();

    void applyTransformations();
    void handleMousePress(QMouseEvent *event);
    void handleMouseMove(QMouseEvent *event);
    void handleWheel(QWheelEvent *event);

private:
    float rotationX;
    float rotationY;
    float zoomDistance;
    QPoint lastMousePosition;

    float targetX;
    float targetZ;
};

#endif // CAMERAMANAGER_H

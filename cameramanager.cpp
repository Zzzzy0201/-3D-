#include "cameramanager.h"
#include <QtMath>

CameraManager::CameraManager()
{
    rotationX = 30.0f;
    rotationY = 0.0f;
    zoomDistance = -500.0f;

    targetX = 0.0f;
    targetZ=0.0f;
}

void CameraManager::applyTransformations()
{
    glTranslatef(0.0f, 0.0f, zoomDistance);
    glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    glTranslatef(-targetX, 0.0f, -targetZ);
}

void CameraManager::handleMousePress(QMouseEvent *event)
{
    lastMousePosition = event->pos();
}

void CameraManager::handleMouseMove(QMouseEvent *event)
{
    int dx = event->position().x() - lastMousePosition.x();
    int dy = event->position().y() - lastMousePosition.y();

    if (event->buttons() & Qt::LeftButton) {
        rotationY += 0.5f * dx;
        rotationX += 0.5f * dy;
        if (rotationX > 85.0f) rotationX = 85.0f;
        if (rotationX < 5.0f) rotationX = 5.0f;
    }
    else if (event->buttons() & Qt::RightButton) {
        float radians = qDegreesToRadians(rotationY);
        float sinY = qSin(radians);
        float cosY = qCos(radians);
        float speedFactor = 0.5f;
        targetX -= (dx * cosY - dy * sinY) * speedFactor;
        targetZ -= (dx * sinY + dy * cosY) * speedFactor;
    }
    lastMousePosition = event->pos();
}

void CameraManager::handleWheel(QWheelEvent *event)
{
    zoomDistance += (event->angleDelta().y() / 120.0f) * 30.0f;

    if (zoomDistance > -50.0f) zoomDistance = -50.0f;
    if (zoomDistance < -1500.0f) zoomDistance = -1500.0f;
}
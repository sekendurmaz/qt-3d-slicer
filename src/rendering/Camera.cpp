#include "Camera.h"
#include <QtMath>

namespace rendering {

Camera::Camera()
    : target_(0, 0, 0)
    , distance_(100.0f)
    , angleX_(45.0f)
    , angleY_(30.0f)
{
}

QMatrix4x4 Camera::viewMatrix() const
{
    QMatrix4x4 view;

    // Camera position (spherical coordinates)
    float radX = qDegreesToRadians(angleX_);
    float radY = qDegreesToRadians(angleY_);

    float x = distance_ * qCos(radY) * qSin(radX);
    float y = distance_ * qSin(radY);
    float z = distance_ * qCos(radY) * qCos(radX);

    QVector3D eye = target_ + QVector3D(x, y, z);
    QVector3D up(0, 1, 0);

    view.lookAt(eye, target_, up);

    return view;
}

QMatrix4x4 Camera::projectionMatrix(float aspect) const
{
    QMatrix4x4 projection;
    projection.perspective(45.0f, aspect, 0.1f, 10000.0f);
    return projection;
}

void Camera::rotate(float deltaX, float deltaY)
{
    angleX_ += deltaX;
    angleY_ += deltaY;

    // Clamp Y rotation
    if (angleY_ > 89.0f) angleY_ = 89.0f;
    if (angleY_ < -89.0f) angleY_ = -89.0f;
}

void Camera::zoom(float delta)
{
    distance_ -= delta;

    // Clamp distance
    if (distance_ < minDistance_) distance_ = minDistance_;
    if (distance_ > maxDistance_) distance_ = maxDistance_;
}

void Camera::pan(float deltaX, float deltaY)
{
    // Pan in camera space
    float radX = qDegreesToRadians(angleX_);

    QVector3D right(qCos(radX), 0, -qSin(radX));
    QVector3D up(0, 1, 0);

    float panSpeed = distance_ * 0.001f;

    target_ += right * deltaX * panSpeed;
    target_ += up * deltaY * panSpeed;
}

void Camera::reset()
{
    distance_ = 100.0f;
    angleX_ = 45.0f;
    angleY_ = 30.0f;
    target_ = QVector3D(0, 0, 0);
}

void Camera::setTarget(const QVector3D& target)
{
    target_ = target;
}

} // namespace rendering

#pragma once

#include <QMatrix4x4>
#include <QVector3D>

namespace rendering {

/**
 * @brief Orbit camera
 */
class Camera
{
public:
    Camera();

    /**
     * @brief View matrix
     */
    QMatrix4x4 viewMatrix() const;

    /**
     * @brief Projection matrix
     */
    QMatrix4x4 projectionMatrix(float aspect) const;

    /**
     * @brief Camera controls
     */
    void rotate(float deltaX, float deltaY);  // Orbit
    void zoom(float delta);                   // Zoom in/out
    void pan(float deltaX, float deltaY);     // Pan

    /**
     * @brief Reset camera
     */
    void reset();

    /**
     * @brief Set target point (model center)
     */
    void setTarget(const QVector3D& target);

private:
    QVector3D target_;      // Look-at point
    float distance_;        // Distance from target
    float angleX_;          // Rotation around Y axis
    float angleY_;          // Rotation around X axis

    float minDistance_ = 1.0f;
    float maxDistance_ = 1000.0f;
};

} // namespace rendering

#pragma once

#include <QVector3D>
#include <QVector2D>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

namespace rendering {

enum class GizmoAxis {
    None,
    X,     // Kırmızı ok (X axis)
    Y,     // Yeşil ok (Y axis)
    Z,     // Mavi ok (Z axis - rotation için)
    XY     // Merkez (free move)
};

class TransformGizmo : protected QOpenGLFunctions
{
public:
    TransformGizmo();
    ~TransformGizmo();

    void initialize();
    void setPosition(const QVector3D& pos);
    void setSize(float size) { size_ = size; }
    void setVisible(bool visible) { visible_ = visible; }

    bool isVisible() const { return visible_; }
    QVector3D position() const { return position_; }

    // Mouse interaction
    GizmoAxis hitTest(const QVector2D& mousePos,
                      const QMatrix4x4& view,
                      const QMatrix4x4& projection,
                      int screenWidth,
                      int screenHeight);

    void startDrag(GizmoAxis axis, const QVector2D& mousePos,
                   const QMatrix4x4& view,
                   const QMatrix4x4& projection,
                   int screenWidth,
                   int screenHeight);

    QVector3D updateDrag(const QVector2D& mousePos,
                         const QMatrix4x4& view,
                         const QMatrix4x4& projection,
                         int screenWidth,
                         int screenHeight);

    // Rendering
    void render(const QMatrix4x4& view,
                const QMatrix4x4& projection,
                QOpenGLShaderProgram* shader);

private:
    QVector3D position_;
    float size_;
    bool visible_;

    GizmoAxis hoveredAxis_;
    GizmoAxis draggedAxis_;
    QVector2D dragStartPos_;
    QVector3D dragStartWorldPos_;

    QVector3D worldToScreen(const QVector3D& worldPos,
                            const QMatrix4x4& view,
                            const QMatrix4x4& projection,
                            int screenWidth,
                            int screenHeight);

    QVector3D screenToWorld(const QVector2D& screenPos,
                            float depth,
                            const QMatrix4x4& view,
                            const QMatrix4x4& projection,
                            int screenWidth,
                            int screenHeight);
};

} // namespace rendering

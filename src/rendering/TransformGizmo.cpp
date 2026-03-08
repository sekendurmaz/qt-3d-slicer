#include "TransformGizmo.h"
#include <QOpenGLShaderProgram>
#include <QDebug>
#include <cmath>

namespace rendering {

TransformGizmo::TransformGizmo()
    : position_(0, 0, 0),
      size_(20.0f),
      visible_(false),
      hoveredAxis_(GizmoAxis::None),
      draggedAxis_(GizmoAxis::None)
{
}

TransformGizmo::~TransformGizmo()
{
}

void TransformGizmo::initialize()
{
    initializeOpenGLFunctions();
}

void TransformGizmo::setPosition(const QVector3D& pos)
{
    position_ = pos;
}

GizmoAxis TransformGizmo::hitTest(const QVector2D& mousePos,
                                  const QMatrix4x4& view,
                                  const QMatrix4x4& projection,
                                  int screenWidth,
                                  int screenHeight)
{
    if (!visible_)
        return GizmoAxis::None;

    // Gizmo merkezini ekran koordinatlarına çevir
    QMatrix4x4 mvp = projection * view;
    QVector4D clipPos = mvp * QVector4D(position_, 1.0f);

    if (clipPos.w() <= 0.001f)
    {
        qDebug() << "⚠️ Gizmo behind camera!";
        return GizmoAxis::None;
    }

    QVector3D ndc = clipPos.toVector3D() / clipPos.w();

    float centerX = (ndc.x() + 1.0f) * 0.5f * screenWidth;
    float centerY = (1.0f - ndc.y()) * 0.5f * screenHeight;

    // X axis (kırmızı ok - sağa 60px)
    QVector2D xArrowTip(centerX + 60, centerY);
    float distX = (mousePos - xArrowTip).length();
    if (distX < 20.0f) return GizmoAxis::X;

    // Y axis (yeşil ok - yukarı 60px)
    QVector2D yArrowTip(centerX, centerY - 60);
    float distY = (mousePos - yArrowTip).length();
    if (distY < 20.0f) return GizmoAxis::Y;

    // Z axis (mavi ok - sağ üst çapraz 80px, 30px) ← YENİ!
    QVector2D zArrowTip(centerX + 80, centerY - 30);
    float distZ = (mousePos - zArrowTip).length();
    if (distZ < 20.0f) return GizmoAxis::Z;

    // Merkez (sarı nokta)
    QVector2D center(centerX, centerY);
    float distCenter = (mousePos - center).length();
    if (distCenter < 15.0f) return GizmoAxis::XY;

    return GizmoAxis::None;
}

void TransformGizmo::startDrag(GizmoAxis axis,
                                const QVector2D& mousePos,
                                const QMatrix4x4& view,
                                const QMatrix4x4& projection,
                                int screenWidth,
                                int screenHeight)
{
    draggedAxis_ = axis;
    dragStartPos_ = mousePos;
    dragStartWorldPos_ = position_;
}

QVector3D TransformGizmo::updateDrag(const QVector2D& mousePos,
                                     const QMatrix4x4& view,
                                     const QMatrix4x4& projection,
                                     int screenWidth,
                                     int screenHeight)
{
    QVector2D delta = mousePos - dragStartPos_;

    // Ekran hareketini dünya hareketine çevir
    float scaleFactor = 0.5f;  // Sensitivity

    QVector3D movement(0, 0, 0);

    if (draggedAxis_ == GizmoAxis::X)
    {
        movement.setX(delta.x() * scaleFactor);
    }
    else if (draggedAxis_ == GizmoAxis::Y)
    {
        movement.setY(-delta.y() * scaleFactor);  // Y inverted
    }
    else if (draggedAxis_ == GizmoAxis::Z)  // ← YENİ!
    {
        movement.setZ(-delta.y() * scaleFactor);  // Mouse Y → Z hareket
    }
    else if (draggedAxis_ == GizmoAxis::XY)
    {
        movement.setX(delta.x() * scaleFactor);
        movement.setY(-delta.y() * scaleFactor);
    }

    position_ = dragStartWorldPos_ + movement;
    dragStartPos_ = mousePos;
    dragStartWorldPos_ = position_;

    return movement;
}

void TransformGizmo::render(const QMatrix4x4& view,
                            const QMatrix4x4& projection,
                            QOpenGLShaderProgram* shader)
{
    if (!visible_)
        return;

}



QVector3D TransformGizmo::screenToWorld(const QVector2D& screenPos,
                                         float depth,
                                         const QMatrix4x4& view,
                                         const QMatrix4x4& projection,
                                         int screenWidth,
                                         int screenHeight)
{
    // TODO: Implement if needed
    return QVector3D(0, 0, 0);
}

} // namespace rendering

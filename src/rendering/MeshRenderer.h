#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QWheelEvent>

#include "core/mesh/mesh.h"
#include "Camera.h"

namespace rendering {

/**
 * @brief OpenGL mesh renderer
 */
class MeshRenderer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit MeshRenderer(QWidget* parent = nullptr);
    ~MeshRenderer() override;

    /**
     * @brief Render edilecek mesh'i set et
     */
    void setMesh(const core::mesh::Mesh& mesh);

    /**
     * @brief Render mode
     */
    enum class RenderMode {
        Wireframe,
        Solid,
        SolidWireframe
    };

    void setRenderMode(RenderMode mode);

protected:
    // QOpenGLWidget overrides
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // Mouse events
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    // OpenGL resources
    QOpenGLVertexArrayObject vao_;
    QOpenGLBuffer vbo_;
    QOpenGLShaderProgram* shaderProgram_ = nullptr;

    // Mesh data
    std::vector<float> vertices_;  // Vertex data (x,y,z, nx,ny,nz)
    int vertexCount_ = 0;

    // Camera
    Camera camera_;

    // Render state
    RenderMode renderMode_ = RenderMode::Solid;

    // Mouse interaction
    QPoint lastMousePos_;
    bool isRotating_ = false;

    // Helper functions
    void buildVertexBuffer(const core::mesh::Mesh& mesh);
    void createShaders();
};

} // namespace rendering

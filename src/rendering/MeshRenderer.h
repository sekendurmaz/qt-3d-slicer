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
#include "core/slicing/Layer.h"  // ← YENİ!
#include "Camera.h"

namespace rendering {

class MeshRenderer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit MeshRenderer(QWidget* parent = nullptr);
    ~MeshRenderer() override;

    void setMesh(const core::mesh::Mesh& mesh);

    enum class RenderMode {
        Wireframe,
        Solid,
        SolidWireframe,
        Layers         // ← YENİ! Layer rendering mode
    };

    void setRenderMode(RenderMode mode);

    // Layer rendering ← YENİ!
    void setLayers(const std::vector<core::slicing::Layer>& layers);
    void setCurrentLayer(int layerIndex);  // -1 = show all
    void clearLayers();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    // Mesh OpenGL resources
    QOpenGLVertexArrayObject vao_;
    QOpenGLBuffer vbo_;
    QOpenGLShaderProgram* shaderProgram_ = nullptr;

    std::vector<float> vertices_;
    int vertexCount_ = 0;

    // Layer OpenGL resources ← YENİ!
    QOpenGLVertexArrayObject layerVao_;
    QOpenGLBuffer layerVbo_;
    QOpenGLShaderProgram* layerShaderProgram_ = nullptr;

    std::vector<float> layerVertices_;
    int layerVertexCount_ = 0;
    std::vector<core::slicing::Layer> layers_;
    int currentLayerIndex_ = -1;  // -1 = show all

    // Camera
    Camera camera_;

    // Render state
    RenderMode renderMode_ = RenderMode::Solid;

    // Mouse interaction
    QPoint lastMousePos_;
    bool isRotating_ = false;

    // Helper functions
    void buildVertexBuffer(const core::mesh::Mesh& mesh);
    void buildLayerBuffer();  // ← YENİ!
    void createShaders();
    void createLayerShaders();  // ← YENİ!
};

} // namespace rendering

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QWheelEvent>
#include "core/buildplate/BuildPlate.h"
#include <cfloat>  // ← EKLE! (FLT_MAX için)
#include "core/mesh/mesh.h"  // ← EKLE! (Mesh için)
#include "TransformGizmo.h"

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

    void setBuildPlate(std::shared_ptr<core::buildplate::BuildPlate> plate);
    void clearBuildPlate();
    void resetCamera();


    // Transform controls ← YENİ!
    void setModelTranslation(float x, float y, float z);
    void setModelRotation(float x, float y, float z);
    void resetModelTransform();
    void centerModel(const core::mesh::Mesh& mesh);
    QVector3D getModelTranslation() const { return modelTranslation_; }
    QVector3D getModelRotation() const { return modelRotation_; }

    // Selection & Gizmo ← EKLE!
    void setMeshSelected(bool selected);
    bool isMeshSelected() const { return meshSelected_; }

signals:
    void modelTransformed(QVector3D translation, QVector3D rotation);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;



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
     QOpenGLShaderProgram* plateShaderProgram_ = nullptr;

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
    void createPlateShaders();

    std::shared_ptr<core::buildplate::BuildPlate> buildPlate_;
    QOpenGLBuffer buildPlateVBO_;
    QOpenGLVertexArrayObject buildPlateVAO_;
    int buildPlateVertexCount_ = 0;
    // Rendering helpers - YENİ!
    void renderBuildPlate();
    void setupBuildPlateBuffers();

    // Model transform
    QVector3D modelTranslation_;
    QVector3D modelRotation_;

    // Gizmo ← EKLE!
    TransformGizmo gizmo_;
    bool meshSelected_;
    GizmoAxis activeGizmoAxis_;
    bool isDraggingGizmo_;
    bool isRotatingCamera_;



};

} // namespace rendering

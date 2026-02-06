#include "MeshRenderer.h"
#include "core/slicing/Layer.h"
#include <QVector3D>

namespace rendering {

MeshRenderer::MeshRenderer(QWidget* parent)
    : QOpenGLWidget(parent)
    , vbo_(QOpenGLBuffer::VertexBuffer)
    , layerVbo_(QOpenGLBuffer::VertexBuffer)  // ← YENİ!
{
    // Enable mouse tracking
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

MeshRenderer::~MeshRenderer()
{
    makeCurrent();
    vbo_.destroy();
    layerVbo_.destroy();  // ← YENİ!
    delete shaderProgram_;
    delete layerShaderProgram_;  // ← YENİ!
    doneCurrent();
}

void MeshRenderer::setMesh(const core::mesh::Mesh& mesh)
{
    makeCurrent();
    buildVertexBuffer(mesh);
    doneCurrent();
    update();
}

void MeshRenderer::setRenderMode(RenderMode mode)
{
    renderMode_ = mode;
    update();
}

void MeshRenderer::initializeGL()
{
    initializeOpenGLFunctions();

    // Background color
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Depth test
    glEnable(GL_DEPTH_TEST);

    // Create VAO
    vao_.create();
    vao_.bind();

    // Create VBO
    vbo_.create();

    // Create shaders
    createShaders();

    vao_.release();

    // Layer VAO/VBO ← YENİ!
    layerVao_.create();
    layerVao_.bind();
    layerVbo_.create();

    createLayerShaders();

    layerVao_.release();
}

void MeshRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void MeshRenderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 model;
    QMatrix4x4 view = camera_.viewMatrix();
    QMatrix4x4 projection = camera_.projectionMatrix(
        static_cast<float>(width()) / static_cast<float>(height())
        );

    // Render layers (if layer mode) ← YENİ!
    if (renderMode_ == RenderMode::Layers && layerVertexCount_ > 0)
    {
        layerShaderProgram_->bind();
        layerShaderProgram_->setUniformValue("model", model);
        layerShaderProgram_->setUniformValue("view", view);
        layerShaderProgram_->setUniformValue("projection", projection);

        layerVao_.bind();
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, layerVertexCount_);
        glLineWidth(1.0f);
        layerVao_.release();

        layerShaderProgram_->release();
        return;  // Don't render mesh
    }

    // Normal mesh rendering
    if (vertexCount_ == 0)
        return;

    shaderProgram_->bind();

    shaderProgram_->setUniformValue("model", model);
    shaderProgram_->setUniformValue("view", view);
    shaderProgram_->setUniformValue("projection", projection);
    shaderProgram_->setUniformValue("lightDir", QVector3D(0.5f, 1.0f, 0.3f).normalized());

    vao_.bind();

    if (renderMode_ == RenderMode::Wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else if (renderMode_ == RenderMode::Solid)
    {
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    }
    else // SolidWireframe
    {
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1, -1);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    vao_.release();
    shaderProgram_->release();
}

void MeshRenderer::mousePressEvent(QMouseEvent* event)
{
    lastMousePos_ = event->pos();

    if (event->button() == Qt::LeftButton)
    {
        isRotating_ = true;
    }
}

void MeshRenderer::mouseMoveEvent(QMouseEvent* event)
{
    int dx = event->pos().x() - lastMousePos_.x();
    int dy = event->pos().y() - lastMousePos_.y();

    if (isRotating_ && (event->buttons() & Qt::LeftButton))
    {
        camera_.rotate(dx * 0.5f, -dy * 0.5f);
        update();
    }
    else if (event->buttons() & Qt::MiddleButton)
    {
        camera_.pan(-dx, dy);
        update();
    }

    lastMousePos_ = event->pos();
}

void MeshRenderer::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        isRotating_ = false;
    }
}

void MeshRenderer::wheelEvent(QWheelEvent* event)
{
    float delta = event->angleDelta().y() / 10.0f;
    camera_.zoom(delta);
    update();
}

void MeshRenderer::buildVertexBuffer(const core::mesh::Mesh& mesh)
{
    vertices_.clear();

    // Each triangle: 3 vertices, each vertex: position (3) + normal (3)
    vertices_.reserve(mesh.triangles.size() * 3 * 6);

    for (const auto& tri : mesh.triangles)
    {
        // Vertex 1
        vertices_.push_back(tri.vertex1.x);
        vertices_.push_back(tri.vertex1.y);
        vertices_.push_back(tri.vertex1.z);
        vertices_.push_back(tri.normal.x);
        vertices_.push_back(tri.normal.y);
        vertices_.push_back(tri.normal.z);

        // Vertex 2
        vertices_.push_back(tri.vertex2.x);
        vertices_.push_back(tri.vertex2.y);
        vertices_.push_back(tri.vertex2.z);
        vertices_.push_back(tri.normal.x);
        vertices_.push_back(tri.normal.y);
        vertices_.push_back(tri.normal.z);

        // Vertex 3
        vertices_.push_back(tri.vertex3.x);
        vertices_.push_back(tri.vertex3.y);
        vertices_.push_back(tri.vertex3.z);
        vertices_.push_back(tri.normal.x);
        vertices_.push_back(tri.normal.y);
        vertices_.push_back(tri.normal.z);
    }

    vertexCount_ = static_cast<int>(mesh.triangles.size() * 3);

    // Upload to GPU
    vao_.bind();
    vbo_.bind();
    vbo_.allocate(vertices_.data(), vertices_.size() * sizeof(float));

    // Position attribute
    shaderProgram_->enableAttributeArray(0);
    shaderProgram_->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));

    // Normal attribute
    shaderProgram_->enableAttributeArray(1);
    shaderProgram_->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));

    vbo_.release();
    vao_.release();
}

void MeshRenderer::createShaders()
{
    shaderProgram_ = new QOpenGLShaderProgram(this);

    // Vertex shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 normal;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec3 fragNormal;
        out vec3 fragPos;

        void main()
        {
            gl_Position = projection * view * model * vec4(position, 1.0);
            fragNormal = mat3(transpose(inverse(model))) * normal;
            fragPos = vec3(model * vec4(position, 1.0));
        }
    )";

    // Fragment shader
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 fragNormal;
        in vec3 fragPos;

        uniform vec3 lightDir;

        out vec4 FragColor;

        void main()
        {
            // Simple diffuse lighting
            vec3 norm = normalize(fragNormal);
            vec3 lightDirection = normalize(lightDir);

            float diff = max(dot(norm, lightDirection), 0.0);

            vec3 baseColor = vec3(0.3, 0.6, 0.9);  // Nice blue
            vec3 ambient = 0.3 * baseColor;
            vec3 diffuse = diff * baseColor;

            vec3 result = ambient + diffuse;
            FragColor = vec4(result, 1.0);
        }
    )";

    shaderProgram_->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    shaderProgram_->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    shaderProgram_->link();
}

void MeshRenderer::setLayers(const std::vector<core::slicing::Layer>& layers)
{
    makeCurrent();
    layers_ = layers;
    buildLayerBuffer();
    doneCurrent();
    update();
}

void MeshRenderer::setCurrentLayer(int layerIndex)
{
    currentLayerIndex_ = layerIndex;
    makeCurrent();
    buildLayerBuffer();
    doneCurrent();
    update();
}

void MeshRenderer::clearLayers()
{
    layers_.clear();
    layerVertices_.clear();
    layerVertexCount_ = 0;
    update();
}

void MeshRenderer::buildLayerBuffer()
{
    layerVertices_.clear();

    if (layers_.empty())
    {
        layerVertexCount_ = 0;
        return;
    }

    // Determine which layers to render
    size_t startLayer = 0;
    size_t endLayer = layers_.size();

    if (currentLayerIndex_ >= 0 && currentLayerIndex_ < static_cast<int>(layers_.size()))
    {
        // Single layer
        startLayer = currentLayerIndex_;
        endLayer = currentLayerIndex_ + 1;
    }

    // Build vertex data (position + color)
    for (size_t i = startLayer; i < endLayer; ++i)
    {
        const auto& layer = layers_[i];

        // Color gradient (blue to red based on Z height)
        float t = static_cast<float>(i) / static_cast<float>(layers_.size());
        float r = t;
        float g = 0.3f;
        float b = 1.0f - t;

        for (const auto& segment : layer.segments())
        {
            // Start point
            layerVertices_.push_back(segment.start.x);
            layerVertices_.push_back(segment.start.y);
            layerVertices_.push_back(segment.start.z);
            layerVertices_.push_back(r);
            layerVertices_.push_back(g);
            layerVertices_.push_back(b);

            // End point
            layerVertices_.push_back(segment.end.x);
            layerVertices_.push_back(segment.end.y);
            layerVertices_.push_back(segment.end.z);
            layerVertices_.push_back(r);
            layerVertices_.push_back(g);
            layerVertices_.push_back(b);
        }
    }

    layerVertexCount_ = static_cast<int>(layerVertices_.size() / 6);

    // Upload to GPU
    layerVao_.bind();
    layerVbo_.bind();
    layerVbo_.allocate(layerVertices_.data(), layerVertices_.size() * sizeof(float));

    // Position attribute
    layerShaderProgram_->enableAttributeArray(0);
    layerShaderProgram_->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));

    // Color attribute
    layerShaderProgram_->enableAttributeArray(1);
    layerShaderProgram_->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));

    layerVbo_.release();
    layerVao_.release();
}

void MeshRenderer::createLayerShaders()
{
    layerShaderProgram_ = new QOpenGLShaderProgram(this);

    // Vertex shader (simple, no lighting)
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec3 fragColor;

        void main()
        {
            gl_Position = projection * view * model * vec4(position, 1.0);
            fragColor = color;
        }
    )";

    // Fragment shader
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 fragColor;
        out vec4 FragColor;

        void main()
        {
            FragColor = vec4(fragColor, 1.0);
        }
    )";

    layerShaderProgram_->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    layerShaderProgram_->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    layerShaderProgram_->link();
}

} // namespace rendering

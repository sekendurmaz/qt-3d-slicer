#include "MeshRenderer.h"
#include <QVector3D>

namespace rendering {

MeshRenderer::MeshRenderer(QWidget* parent)
    : QOpenGLWidget(parent)
    , vbo_(QOpenGLBuffer::VertexBuffer)
{
    // Enable mouse tracking
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

MeshRenderer::~MeshRenderer()
{
    makeCurrent();
    vbo_.destroy();
    delete shaderProgram_;
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
}

void MeshRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void MeshRenderer::paintGL()
{
    // Clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (vertexCount_ == 0)
        return;

    // Use shader
    shaderProgram_->bind();

    // Set matrices
    QMatrix4x4 model;
    QMatrix4x4 view = camera_.viewMatrix();
    QMatrix4x4 projection = camera_.projectionMatrix(
        static_cast<float>(width()) / static_cast<float>(height())
    );

    shaderProgram_->setUniformValue("model", model);
    shaderProgram_->setUniformValue("view", view);
    shaderProgram_->setUniformValue("projection", projection);

    // Light direction
    shaderProgram_->setUniformValue("lightDir", QVector3D(0.5f, 1.0f, 0.3f).normalized());

    // Bind VAO and draw
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
        // Draw solid
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);

        // Draw wireframe on top
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

} // namespace rendering

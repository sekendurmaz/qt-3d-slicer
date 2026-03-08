#include "MeshRenderer.h"
#include "core/slicing/Layer.h"
#include <QVector3D>
//#include "core/buildplate/RectangularPlate.h"
#include "core/buildplate/CircularPlate.h"
#include <cmath>
#include <chrono>
#include <QPainter>

namespace rendering {

MeshRenderer::MeshRenderer(QWidget* parent)
    : QOpenGLWidget(parent)
    , vbo_(QOpenGLBuffer::VertexBuffer)
    , layerVbo_(QOpenGLBuffer::VertexBuffer)
    , modelTranslation_(0.0f, 0.0f, 0.0f)
    , modelRotation_(0.0f, 0.0f, 0.0f)
    , activeGizmoAxis_(GizmoAxis::None)
    , isDraggingGizmo_(false)
    , isRotatingCamera_(false)
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

    buildPlateVAO_.create();
    buildPlateVBO_.create();

    gizmo_.initialize();
    gizmo_.setSize(20.0f);
    gizmo_.setVisible(false);

    qDebug() << "✅ Gizmo initialized";


}

void MeshRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void MeshRenderer::paintGL()
{
    static int frameCount = 0;
    static auto lastTime = std::chrono::high_resolution_clock::now();

    frameCount++;
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime).count();

    if (elapsed >= 1) {  // Her saniye FPS göster
        qDebug() << "🎬 FPS:" << frameCount;
        frameCount = 0;
        lastTime = now;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera matrices
    QMatrix4x4 view = camera_.viewMatrix();
    QMatrix4x4 projection = camera_.projectionMatrix(
        static_cast<float>(width()) / static_cast<float>(height())
        );

    // ============================================
    // 1. BUILD PLATE (SABİT - NO TRANSFORM!) ← DEĞİŞTİ!
    // ============================================
    if (buildPlate_)
    {
        QMatrix4x4 plateModel;  // Identity matrix (transform YOK!)

        // Build plate shader'ı varsa kullan
        // Yoksa renderBuildPlate() kendi shader'ını kullanır
        renderBuildPlate();  // Bu fonksiyon kendi shader'ını handle ediyor olabilir
    }

    // Model transform matrix ← YENİ!
    QMatrix4x4 model;
    model.translate(modelTranslation_);

    // 3 EKSEN ROTASYON! ← YENİ!
    model.rotate(modelRotation_.z(), 0, 0, 1);  // Z rotation
    model.rotate(modelRotation_.y(), 0, 1, 0);  // Y rotation
    model.rotate(modelRotation_.x(), 1, 0, 0);  // X rotation

    // ============================================
    // 3. LAYERS RENDERING (TRANSFORM UYGULANIR)
    // ============================================
    if (renderMode_ == RenderMode::Layers && layerVertexCount_ > 0)
    {
        layerShaderProgram_->bind();
        layerShaderProgram_->setUniformValue("model", model);  // ← TRANSFORM UYGULANIR
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

    // ============================================
    // 4. MESH RENDERING (TRANSFORM UYGULANIR)
    // ============================================
    if (vertexCount_ == 0)
        return;

    shaderProgram_->bind();

    shaderProgram_->setUniformValue("model", model);  // ← TRANSFORM UYGULANIR
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

    // ============================================
    // GIZMO RENDERING (EN SON!) ← EKLE!
    // ============================================
    if (gizmo_.isVisible())
    {
        gizmo_.render(view, projection, shaderProgram_);
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
void MeshRenderer::setBuildPlate(std::shared_ptr<core::buildplate::BuildPlate> plate)
{

    buildPlate_ = plate; 
    setupBuildPlateBuffers();  
    update();



}

void MeshRenderer::clearBuildPlate()
{
    buildPlate_.reset();
    update();
}

void MeshRenderer::setupBuildPlateBuffers()
{
    if (!buildPlate_)
    {
        return;
    }

    makeCurrent();

    std::vector<float> vertices;

    if (buildPlate_->type() == core::buildplate::PlateType::Rectangular)
    {
        // Rectangular plate - grid lines
        float w = buildPlate_->width();
        float d = buildPlate_->depth();
        float gridSpacing = buildPlate_->gridSpacing();

        // Horizontal lines (along X axis)
        int numLinesY = static_cast<int>(d / gridSpacing) + 1;
        for (int i = 0; i < numLinesY; ++i)
        {
            float y = -d/2 + (i * gridSpacing);
            if (y > d/2 + 0.001f) break;  // Float precision guard

            vertices.push_back(-w/2); vertices.push_back(y); vertices.push_back(0.0f);
            vertices.push_back(w/2);  vertices.push_back(y); vertices.push_back(0.0f);
        }

        // Vertical lines (along Y axis)
        int numLinesX = static_cast<int>(w / gridSpacing) + 1;
        for (int i = 0; i < numLinesX; ++i)
        {
            float x = -w/2 + (i * gridSpacing);
            if (x > w/2 + 0.001f) break;  // Float precision guard

            vertices.push_back(x); vertices.push_back(-d/2); vertices.push_back(0.0f);
            vertices.push_back(x); vertices.push_back(d/2);  vertices.push_back(0.0f);
        }

        // Border (thicker)
        vertices.push_back(-w/2); vertices.push_back(-d/2); vertices.push_back(0.0f);
        vertices.push_back(w/2);  vertices.push_back(-d/2); vertices.push_back(0.0f);

        vertices.push_back(w/2);  vertices.push_back(-d/2); vertices.push_back(0.0f);
        vertices.push_back(w/2);  vertices.push_back(d/2);  vertices.push_back(0.0f);

        vertices.push_back(w/2);  vertices.push_back(d/2);  vertices.push_back(0.0f);
        vertices.push_back(-w/2); vertices.push_back(d/2);  vertices.push_back(0.0f);

        vertices.push_back(-w/2); vertices.push_back(d/2);  vertices.push_back(0.0f);
        vertices.push_back(-w/2); vertices.push_back(-d/2); vertices.push_back(0.0f);
    }
    else // Circular
    {
        auto circPlate = std::static_pointer_cast<core::buildplate::CircularPlate>(buildPlate_);
        float r = circPlate->radius();
        int segments = 64;

        // Circle border
        for (int i = 0; i <= segments; ++i)
        {
            float angle1 = 2.0f * M_PI * i / segments;
            float angle2 = 2.0f * M_PI * (i + 1) / segments;

            vertices.push_back(r * std::cos(angle1));
            vertices.push_back(r * std::sin(angle1));
            vertices.push_back(0.0f);

            vertices.push_back(r * std::cos(angle2));
            vertices.push_back(r * std::sin(angle2));
            vertices.push_back(0.0f);
        }

        // Cross lines
        vertices.push_back(-r); vertices.push_back(0); vertices.push_back(0.0f);
        vertices.push_back(r);  vertices.push_back(0); vertices.push_back(0.0f);

        vertices.push_back(0); vertices.push_back(-r); vertices.push_back(0.0f);
        vertices.push_back(0); vertices.push_back(r);  vertices.push_back(0.0f);
    }

    buildPlateVertexCount_ = vertices.size() / 3;

    qDebug() << "🔧 Build plate buffer created:" << vertices.size() << "floats," << (vertices.size()/3) << "vertices";  // ← DEBUG

    buildPlateVAO_.bind();
    buildPlateVBO_.bind();
    buildPlateVBO_.allocate(vertices.data(), vertices.size() * sizeof(float));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    buildPlateVAO_.release();
    buildPlateVBO_.release();

    doneCurrent();
}


void MeshRenderer::renderBuildPlate()
{
    static int callCount = 0;
    static auto lastLog = std::chrono::high_resolution_clock::now();

    auto start = std::chrono::high_resolution_clock::now();

    if (!buildPlate_)
    {
        return;
    }

    if (!shaderProgram_->bind())
    {
        return;
    }

    // Setup matrices - BUILD PLATE İÇİN SABİT! ← DEĞİŞTİ!
    QMatrix4x4 model;
    model.setToIdentity();  // NO ROTATION, NO TRANSLATION (sadece mesh transform'u)
    // model.translate(0, 0, 0.01f);  // ← KALDIR! (Z-fighting için gereksiz)

    float aspect = width() / static_cast<float>(height());

    shaderProgram_->setUniformValue("model", model);  // ← Identity matrix
    shaderProgram_->setUniformValue("view", camera_.viewMatrix());
    shaderProgram_->setUniformValue("projection", camera_.projectionMatrix(aspect));
    shaderProgram_->setUniformValue("meshColor", QVector3D(0.3f, 0.3f, 0.3f));  // ← Gri yap (yeşil kafa karıştırıcı)

    // Draw grid
    buildPlateVAO_.bind();
    glLineWidth(1.5f);  // ← Biraz ince

    glDrawArrays(GL_LINES, 0, buildPlateVertexCount_);

    buildPlateVAO_.release();
    shaderProgram_->release();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    callCount++;
    auto now = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastLog).count() >= 1) {
        qDebug() << "🎨 renderBuildPlate avg:" << (duration) << "µs, vertices:" << buildPlateVertexCount_;
        callCount = 0;
        lastLog = now;
    }
}

void MeshRenderer::resetCamera()
{
    camera_.reset();
    update();
}
void MeshRenderer::setModelTranslation(float x, float y, float z)
{
    modelTranslation_ = QVector3D(x, y, z);
    gizmo_.setPosition(modelTranslation_);
    update();
}

void MeshRenderer::setModelRotation(float x, float y, float z)
{
    modelRotation_ = QVector3D(x, y, z);
    update();
}

void MeshRenderer::resetModelTransform()
{
    modelTranslation_ = QVector3D(0.0f, 0.0f, 0.0f);
    modelRotation_ = QVector3D(0, 0, 0);
    update();
    qDebug() << "🔄 Transform reset";
}

void MeshRenderer::centerModel(const core::mesh::Mesh& mesh)
{
    if (mesh.triangles.empty())
        return;

    float minX = FLT_MAX, maxX = -FLT_MAX;
    float minY = FLT_MAX, maxY = -FLT_MAX;

    // Calculate bounds
    for (const auto& tri : mesh.triangles)
    {
        // vertex1, vertex2, vertex3 kullan ← DOĞRU HAL!
        minX = std::min({minX, tri.vertex1.x, tri.vertex2.x, tri.vertex3.x});
        maxX = std::max({maxX, tri.vertex1.x, tri.vertex2.x, tri.vertex3.x});
        minY = std::min({minY, tri.vertex1.y, tri.vertex2.y, tri.vertex3.y});
        maxY = std::max({maxY, tri.vertex1.y, tri.vertex2.y, tri.vertex3.y});
    }

    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;

    modelTranslation_.setX(-centerX);
    modelTranslation_.setY(-centerY);

    update();
    qDebug() << "📍 Model centered at:" << -centerX << "," << -centerY;
}

void MeshRenderer::mousePressEvent(QMouseEvent* event)
{
    lastMousePos_ = event->pos();

    if (event->button() == Qt::LeftButton)
    {
        // Gizmo kontrolü
        if (gizmo_.isVisible())
        {
            QMatrix4x4 view = camera_.viewMatrix();
            QMatrix4x4 projection = camera_.projectionMatrix(
                static_cast<float>(width()) / static_cast<float>(height())
                );

            activeGizmoAxis_ = gizmo_.hitTest(
                QVector2D(event->pos()),  // ← QPoint → QVector2D dönüşümü
                view,
                projection,
                width(),
                height()
                );

            if (activeGizmoAxis_ != GizmoAxis::None)
            {
                // Gizmo oka tıklandı!
                isDraggingGizmo_ = true;

                // startDrag() - 6 PARAMETRE! ← DÜZELT!
                gizmo_.startDrag(
                    activeGizmoAxis_,
                    QVector2D(event->pos()),  // ← QPoint → QVector2D
                    view,                     // ← EKLE!
                    projection,               // ← EKLE!
                    width(),                  // ← EKLE!
                    height()                  // ← EKLE!
                    );

                // Debug: Hangi eksen seçildi?
                if (activeGizmoAxis_ == GizmoAxis::X) {
                    qDebug() << "🔴 X axis selected!";
                } else if (activeGizmoAxis_ == GizmoAxis::Y) {
                    qDebug() << "🟢 Y axis selected!";
                } else if (activeGizmoAxis_ == GizmoAxis::Z) {
                    qDebug() << "🔵 Z axis selected!";
                } else if (activeGizmoAxis_ == GizmoAxis::XY) {
                    qDebug() << "🟡 XY center selected!";
                }

                return;
            }
        }

        // Mesh seçimi
        if (vertexCount_ > 0)
        {
            setMeshSelected(true);
            qDebug() << "✅ Mesh selected!";
            update();
            return;
        }
        else
        {
            // Boşa tıklandı
            setMeshSelected(false);
            update();
        }
    }

    // Kamera rotasyonu (right click)
    if (event->button() == Qt::RightButton)
    {
        isRotatingCamera_ = true;
    }
}
void MeshRenderer::mouseMoveEvent(QMouseEvent* event)
{
    QPoint delta = event->pos() - lastMousePos_;

    if (isDraggingGizmo_)
    {


        // Gizmo ile mesh hareket ettir
        QMatrix4x4 view = camera_.viewMatrix();
        QMatrix4x4 projection = camera_.projectionMatrix(
            static_cast<float>(width()) / static_cast<float>(height())
            );



        QVector3D movement = gizmo_.updateDrag(
            QVector2D(event->pos()),
            view,
            projection,
            width(),
            height()
            );



        modelTranslation_ += movement;
        gizmo_.setPosition(modelTranslation_);




        try {
            emit modelTransformed(modelTranslation_, modelRotation_);

        } catch (...) {
            qDebug() << "❌ EXCEPTION in emit!";
        }

        lastMousePos_ = event->pos();
        update();
        return;
    }

    if (isRotatingCamera_)
    {
        // Kamera döndür
        float sensitivity = 0.5f;
        camera_.rotate(delta.x() * sensitivity, delta.y() * sensitivity);

        lastMousePos_ = event->pos();
        update();
    }
}

void MeshRenderer::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        isDraggingGizmo_ = false;
        activeGizmoAxis_ = GizmoAxis::None;
    }

    if (event->button() == Qt::RightButton)
    {
        isRotatingCamera_ = false;
    }
}

void MeshRenderer::setMeshSelected(bool selected)
{
    meshSelected_ = selected;
    gizmo_.setVisible(selected);

    if (selected)
    {
        // Gizmo'yu mesh pozisyonuna yerleştir
        gizmo_.setPosition(modelTranslation_);
        qDebug() << "🎯 Gizmo positioned at:" << modelTranslation_;
    }
}

void MeshRenderer::paintEvent(QPaintEvent* event)
{
    // 3D render
    QOpenGLWidget::paintEvent(event);

    // 2D Gizmo overlay
    if (gizmo_.isVisible())
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Gizmo pozisyonunu ekran koordinatlarına çevir
        QMatrix4x4 view = camera_.viewMatrix();
        QMatrix4x4 projection = camera_.projectionMatrix(
            static_cast<float>(width()) / static_cast<float>(height())
            );

        QMatrix4x4 mvp = projection * view;
        QVector4D clipPos = mvp * QVector4D(gizmo_.position(), 1.0f);

        if (clipPos.w() <= 0.0f)
            return;

        QVector3D ndc = clipPos.toVector3D() / clipPos.w();

        float screenX = (ndc.x() + 1.0f) * 0.5f * width();
        float screenY = (1.0f - ndc.y()) * 0.5f * height();

        QPointF center(screenX, screenY);

        // ==================== X AXIS (KIRMIZI → SAĞA) ====================
        painter.setPen(QPen(Qt::red, 4));
        painter.drawLine(center, center + QPointF(60, 0));
        painter.setBrush(Qt::red);
        QPolygonF arrowX;
        arrowX << (center + QPointF(60, 0))
               << (center + QPointF(52, -6))
               << (center + QPointF(52, 6));
        painter.drawPolygon(arrowX);

        // X Label
        painter.setPen(Qt::red);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(center + QPointF(65, 5), "X");

        // ==================== Y AXIS (YEŞİL ↑ YUKARI) ====================
        painter.setPen(QPen(Qt::green, 4));
        painter.drawLine(center, center + QPointF(0, -60));
        painter.setBrush(Qt::green);
        QPolygonF arrowY;
        arrowY << (center + QPointF(0, -60))
               << (center + QPointF(-6, -52))
               << (center + QPointF(6, -52));
        painter.drawPolygon(arrowY);

        // Y Label
        painter.setPen(Qt::green);
        painter.drawText(center + QPointF(5, -65), "Y");

        // ==================== Z AXIS (MAVİ ↗ ÇAPRAZ) ← YENİ! ====================
        painter.setPen(QPen(Qt::blue, 4));
        QPointF zEnd = center + QPointF(80, -30);  // Sağ üst çapraz
        painter.drawLine(center, zEnd);
        painter.setBrush(Qt::blue);

        // Mavi ok ucu
        QPolygonF arrowZ;
        arrowZ << zEnd
               << (zEnd + QPointF(-8, 2))   // Sol alt
               << (zEnd + QPointF(-4, -6)); // Üst
        painter.drawPolygon(arrowZ);

        // Z Label
        painter.setPen(Qt::blue);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(zEnd + QPointF(8, 5), "Z");

        // ==================== MERKEZ (SARI ●) ====================
        painter.setPen(QPen(Qt::yellow, 2));
        painter.setBrush(Qt::yellow);
        painter.drawEllipse(center, 10, 10);

        // Merkez Label
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.drawText(center + QPointF(-5, -15), "XY");
    }
}
} // namespace rendering

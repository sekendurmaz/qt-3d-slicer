#include "mainwindow.h"
#include "rendering/MeshRenderer.h"
#include "io/models/common/ModelFactory.h"
#include "core/mesh/MeshValidator.h"
#include "core/mesh/MeshAnalyzer.h"
#include "core/mesh/NormalProcessor.h"
#include "core/mesh/MeshRepairer.h"
#include "core/slicing/Slicer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QFileInfo>
#include <QLabel>
#include "QSlider"
#include <QDebug>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <chrono>

// Arka plan thread sonuçları için yapılar
namespace {

struct AsyncLoadResult {
    core::mesh::Mesh mesh;
    core::mesh::ValidationResult validResult;
    core::mesh::MeshStatistics stats;
    long long durationMs = 0;
};

struct AsyncRepairResult {
    core::mesh::Mesh mesh;
    core::mesh::MeshRepairResult result;
};

struct AsyncNormalResult {
    core::mesh::Mesh mesh;
    core::mesh::NormalProcessingResult result;
};

} // anonymous namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Window setup
    setWindowTitle("3D Model Viewer & Analyzer");
    resize(1024, 768);

    // Central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main layout (vertical)
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // OpenGL renderer widget
    meshRenderer_ = new rendering::MeshRenderer(this);
    meshRenderer_->setMinimumSize(800, 600);
    mainLayout->addWidget(meshRenderer_, 1); // stretch = 1

    /* =========================== BUTTON LAYOUTS ===========================*/

    // First row: Main controls

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);

    // Buttons
    btnLoad_ = new QPushButton("Load Model", this);
    btnWireframe_ = new QPushButton("Wireframe", this);
    btnSolid_ = new QPushButton("Solid", this);
    btnReset_ = new QPushButton("Reset View", this);

    btnLoad_->setMinimumHeight(35);
    btnWireframe_->setMinimumHeight(35);
    btnSolid_->setMinimumHeight(35);
    btnReset_->setMinimumHeight(35);

    buttonLayout->addWidget(btnLoad_);
    buttonLayout->addWidget(btnWireframe_);
    buttonLayout->addWidget(btnSolid_);
    buttonLayout->addWidget(btnReset_);
    buttonLayout->addStretch(); // Push buttons to left

    mainLayout->addLayout(buttonLayout);

    // Second row: Normal Processing + Repair

    QHBoxLayout* buttonLayout2 = new QHBoxLayout();
    buttonLayout2->setSpacing(5);

    btnRecalcNormals_ = new QPushButton("Recalculate Normals", this);
    btnSmoothNormals_ = new QPushButton("Smooth Normals", this);
    btnFlipNormals_ = new QPushButton("Flip Normals", this);
    btnRepairMesh_ = new QPushButton("🔧 Repair Mesh", this);  // ← YENİ!

    btnRecalcNormals_->setMinimumHeight(35);
    btnSmoothNormals_->setMinimumHeight(35);
    btnFlipNormals_->setMinimumHeight(35);
    btnRepairMesh_->setMinimumHeight(35);  // ← YENİ!

    buttonLayout2->addWidget(btnRecalcNormals_);
    buttonLayout2->addWidget(btnSmoothNormals_);
    buttonLayout2->addWidget(btnFlipNormals_);
    buttonLayout2->addWidget(btnRepairMesh_);  // ← YENİ!
    buttonLayout2->addStretch();

    mainLayout->addLayout(buttonLayout2);

    // Third row: Slicing ← YENİ!
    QHBoxLayout* buttonLayout3 = new QHBoxLayout();
    buttonLayout3->setSpacing(5);

    btnSliceMesh_ = new QPushButton("🍕 Slice Mesh", this);
    btnSliceMesh_->setMinimumHeight(35);
    btnSliceMesh_->setStyleSheet("QPushButton { font-weight: bold; background-color: #4CAF50; color: white; }");

    btnShowLayers_ = new QPushButton("👁️ Show Layers", this);  // ← YENİ!
    btnShowLayers_->setMinimumHeight(35);
    btnShowLayers_->setStyleSheet("QPushButton { font-weight: bold; background-color: #2196F3; color: white; }");
    btnShowLayers_->setEnabled(false);  // Başta disabled

    buttonLayout3->addWidget(btnSliceMesh_);
    buttonLayout3->addWidget(btnShowLayers_);  // ← YENİ!
    buttonLayout3->addStretch();

    mainLayout->addLayout(buttonLayout3);

    // Fourth row: Layer slider ← YENİ!
    QHBoxLayout* sliderLayout = new QHBoxLayout();
    sliderLayout->setSpacing(5);

    QLabel* labelSlider = new QLabel("Layer:", this);
    sliderLayer_ = new QSlider(Qt::Horizontal, this);
    sliderLayer_->setMinimum(0);
    sliderLayer_->setMaximum(0);
    sliderLayer_->setValue(0);
    sliderLayer_->setEnabled(false);

    labelCurrentLayer_ = new QLabel("0 / 0", this);
    labelCurrentLayer_->setMinimumWidth(80);

    sliderLayout->addWidget(labelSlider);
    sliderLayout->addWidget(sliderLayer_, 1);  // stretch
    sliderLayout->addWidget(labelCurrentLayer_);

    mainLayout->addLayout(sliderLayout);

    /* =========================== STATUS BAR WITH LABELS ===========================*/

    // Triangle count label
    labelTriangleCount_ = new QLabel("Triangles: 0", this);
    labelTriangleCount_->setStyleSheet("QLabel { padding: 5px; font-weight: bold; }");
    statusBar()->addPermanentWidget(labelTriangleCount_);

    // Vertex count label
    labelVertexCount_ = new QLabel("Vertices: 0", this);
    labelVertexCount_->setStyleSheet("QLabel { padding: 5px; }");
    statusBar()->addPermanentWidget(labelVertexCount_);

    // Layer count label ← YENİ!
    labelLayerCount_ = new QLabel("Layers: 0", this);
    labelLayerCount_->setStyleSheet("QLabel { padding: 5px; color: #4CAF50; font-weight: bold; }");
    statusBar()->addPermanentWidget(labelLayerCount_);

    // Status bar
    statusBar()->showMessage("Ready - Load a 3D model to begin");

    // Connect signals
    connect(btnLoad_, &QPushButton::clicked, this, &MainWindow::onLoadModel);
    connect(btnWireframe_, &QPushButton::clicked, this, &MainWindow::onWireframe);
    connect(btnSolid_, &QPushButton::clicked, this, &MainWindow::onSolid);
    connect(btnReset_, &QPushButton::clicked, this, &MainWindow::onResetView);

    // Normal processing signals ← YENİ!
    connect(btnRecalcNormals_, &QPushButton::clicked, this, &MainWindow::onRecalculateNormals);
    connect(btnSmoothNormals_, &QPushButton::clicked, this, &MainWindow::onSmoothNormals);
    connect(btnFlipNormals_, &QPushButton::clicked, this, &MainWindow::onFlipNormals);

    // Repair signal ← YENİ!
    connect(btnRepairMesh_, &QPushButton::clicked, this, &MainWindow::onRepairMesh);

    // Slice signals
    connect(btnSliceMesh_, &QPushButton::clicked, this, &MainWindow::onSliceMesh);
    connect(btnShowLayers_, &QPushButton::clicked, this, &MainWindow::onShowLayers);  // ← YENİ!
    connect(sliderLayer_, &QSlider::valueChanged, this, &MainWindow::onLayerChanged);  // ← YENİ!
}

MainWindow::~MainWindow()
{
    // Qt handles cleanup automatically (parent-child relationship)
}

void MainWindow::onLoadModel()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select 3D Model",
        "",
        "3D Models (*.stl *.obj *.3mf);;STL Files (*.stl);;OBJ Files (*.obj);;3MF Files (*.3mf);;All Files (*)"
        );

    if (fileName.isEmpty()) {
        return;
    }

    qDebug() << "Loading:" << fileName;

    setUiBusy(true);
    statusBar()->showMessage("Loading model...");

    auto* watcher = new QFutureWatcher<AsyncLoadResult>(this);
    connect(watcher, &QFutureWatcher<AsyncLoadResult>::finished, this, [this, watcher, fileName]() {
        auto loadResult = watcher->result();
        watcher->deleteLater();

        currentMesh_ = std::move(loadResult.mesh);

        qDebug() << "⏱️  Load time:" << loadResult.durationMs << "ms";

        // Render mesh
        meshRenderer_->setMesh(currentMesh_);

        // UPDATE LABELS
        updateMeshInfo();

        const auto& stats = loadResult.stats;
        const auto& validResult = loadResult.validResult;

        // Update status bar
        QString statusMsg = QString("Loaded: %1 - %2 triangles, Volume: %3 mm³")
                                .arg(QFileInfo(fileName).fileName())
                                .arg(stats.triangleCount)
                                .arg(stats.volume, 0, 'f', 2);
        statusBar()->showMessage(statusMsg);

        // Show detailed info
        QString message;

        message += "=== MODEL STATISTICS ===\n\n";
        message += QString("File: %1\n\n").arg(QFileInfo(fileName).fileName());

        message += QString("Triangles: %1\n").arg(stats.triangleCount);
        message += QString("Vertices: ~%1\n\n").arg(stats.vertexCount);

        message += "=== BOUNDING BOX ===\n";
        message += QString("Min: (%1, %2, %3)\n")
                       .arg(stats.bounds.min.x, 0, 'f', 2)
                       .arg(stats.bounds.min.y, 0, 'f', 2)
                       .arg(stats.bounds.min.z, 0, 'f', 2);
        message += QString("Max: (%1, %2, %3)\n")
                       .arg(stats.bounds.max.x, 0, 'f', 2)
                       .arg(stats.bounds.max.y, 0, 'f', 2)
                       .arg(stats.bounds.max.z, 0, 'f', 2);
        message += QString("Size: %1 x %2 x %3 mm\n\n")
                       .arg(stats.dimensions.x, 0, 'f', 2)
                       .arg(stats.dimensions.y, 0, 'f', 2)
                       .arg(stats.dimensions.z, 0, 'f', 2);

        message += "=== GEOMETRY ===\n";
        message += QString("Surface Area: %1 mm²\n")
                       .arg(stats.surfaceArea, 0, 'f', 2);
        message += QString("Volume: %1 mm³\n")
                       .arg(stats.volume, 0, 'f', 2);
        message += QString("Watertight: %1\n\n")
                       .arg(stats.isWatertight ? "Yes" : "No");

        message += QString("Center: (%1, %2, %3)\n\n")
                       .arg(stats.centerOfMass.x, 0, 'f', 2)
                       .arg(stats.centerOfMass.y, 0, 'f', 2)
                       .arg(stats.centerOfMass.z, 0, 'f', 2);

        message += "=== VALIDATION ===\n";
        message += QString("Status: %1\n").arg(validResult.isValid ? "VALID" : "HAS ERRORS");
        message += QString("Degenerate triangles: %1\n").arg(validResult.degenerateTriangles);
        message += QString("Invalid vertices: %1\n").arg(validResult.invalidVertices);
        message += QString("Duplicate vertices: %1").arg(validResult.duplicateVertices);

        setUiBusy(false);
        QMessageBox::information(this, "Model Analysis", message);

        qDebug() << "Rendered:" << stats.triangleCount << "triangles";
    });

    std::string fileNameStd = fileName.toStdString();
    watcher->setFuture(QtConcurrent::run([fileNameStd]() -> AsyncLoadResult {
        AsyncLoadResult result;

        auto startTime = std::chrono::high_resolution_clock::now();

        result.mesh = io::models::ModelFactory::loadModel(fileNameStd);

        core::mesh::MeshValidator validator;
        result.validResult = validator.validate(result.mesh);

        core::mesh::MeshAnalyzer analyzer;
        result.stats = analyzer.analyze(result.mesh);

        auto endTime = std::chrono::high_resolution_clock::now();
        result.durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                endTime - startTime).count();

        return result;
    }));
}

void MainWindow::onWireframe()
{
    meshRenderer_->setRenderMode(rendering::MeshRenderer::RenderMode::Wireframe);
    statusBar()->showMessage("Render mode: Wireframe");
}

void MainWindow::onSolid()
{
    meshRenderer_->setRenderMode(rendering::MeshRenderer::RenderMode::Solid);
    statusBar()->showMessage("Render mode: Solid");
}

void MainWindow::onResetView()
{
    // Re-set mesh to reset camera
    if (currentMesh_.triangleCount() > 0) {
        meshRenderer_->setMesh(currentMesh_);
        statusBar()->showMessage("View reset");
    }
}

void MainWindow::onRecalculateNormals()
{
    if (currentMesh_.triangleCount() == 0)
    {
        QMessageBox::warning(this, "No Mesh", "Please load a model first.");
        return;
    }

    setUiBusy(true);
    statusBar()->showMessage("Recalculating normals...");

    auto* watcher = new QFutureWatcher<AsyncNormalResult>(this);
    connect(watcher, &QFutureWatcher<AsyncNormalResult>::finished, this, [this, watcher]() {
        auto res = watcher->result();
        watcher->deleteLater();

        currentMesh_ = std::move(res.mesh);
        meshRenderer_->setMesh(currentMesh_);
        updateMeshInfo();

        QString msg = QString("Recalculated %1 normals").arg(res.result.normalsRecalculated);
        statusBar()->showMessage(msg);
        setUiBusy(false);
        QMessageBox::information(this, "Normals Recalculated", msg);
    });

    auto meshCopy = currentMesh_;
    watcher->setFuture(QtConcurrent::run([meshCopy]() mutable -> AsyncNormalResult {
        AsyncNormalResult res;
        core::mesh::NormalProcessor processor;
        res.result = processor.recalculateNormals(meshCopy);
        res.mesh = std::move(meshCopy);
        return res;
    }));
}

void MainWindow::onSmoothNormals()
{
    if (currentMesh_.triangleCount() == 0)
    {
        QMessageBox::warning(this, "No Mesh", "Please load a model first.");
        return;
    }

    setUiBusy(true);
    statusBar()->showMessage("Smoothing normals...");

    auto* watcher = new QFutureWatcher<AsyncNormalResult>(this);
    connect(watcher, &QFutureWatcher<AsyncNormalResult>::finished, this, [this, watcher]() {
        auto res = watcher->result();
        watcher->deleteLater();

        currentMesh_ = std::move(res.mesh);
        meshRenderer_->setMesh(currentMesh_);
        updateMeshInfo();

        QString msg = QString("Smoothed %1 normals (angle threshold: 30°)")
                          .arg(res.result.normalsSmoothed);
        statusBar()->showMessage(msg);
        setUiBusy(false);
        QMessageBox::information(this, "Normals Smoothed", msg);
    });

    auto meshCopy = currentMesh_;
    watcher->setFuture(QtConcurrent::run([meshCopy]() mutable -> AsyncNormalResult {
        AsyncNormalResult res;
        core::mesh::NormalProcessor processor;
        res.result = processor.smoothNormals(meshCopy, 30.0f);
        res.mesh = std::move(meshCopy);
        return res;
    }));
}

void MainWindow::onFlipNormals()
{
    if (currentMesh_.triangleCount() == 0)
    {
        QMessageBox::warning(this, "No Mesh", "Please load a model first.");
        return;
    }

    setUiBusy(true);
    statusBar()->showMessage("Flipping normals...");

    auto* watcher = new QFutureWatcher<AsyncNormalResult>(this);
    connect(watcher, &QFutureWatcher<AsyncNormalResult>::finished, this, [this, watcher]() {
        auto res = watcher->result();
        watcher->deleteLater();

        currentMesh_ = std::move(res.mesh);
        meshRenderer_->setMesh(currentMesh_);
        updateMeshInfo();

        QString msg = QString("Flipped %1 normals").arg(res.result.normalsFlipped);
        statusBar()->showMessage(msg);
        setUiBusy(false);
        QMessageBox::information(this, "Normals Flipped", msg);
    });

    auto meshCopy = currentMesh_;
    watcher->setFuture(QtConcurrent::run([meshCopy]() mutable -> AsyncNormalResult {
        AsyncNormalResult res;
        core::mesh::NormalProcessor processor;
        res.result = processor.flipNormals(meshCopy);
        res.mesh = std::move(meshCopy);
        return res;
    }));
}

void MainWindow::onRepairMesh()
{

    if (currentMesh_.triangleCount() == 0)
    {
        QMessageBox::warning(this, "No Mesh", "Please load a model first.");
        return;
    }

    // Confirm repair
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Repair Mesh",
                                  "This will repair the mesh by:\n"
                                  "- Removing invalid triangles (NaN/Inf)\n"
                                  "- Removing degenerate triangles (area ≈ 0)\n"
                                  "- Merging duplicate vertices\n\n"
                                  "Continue?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No)
    {
        return;
    }

    setUiBusy(true);
    statusBar()->showMessage("Repairing mesh...");

    auto* watcher = new QFutureWatcher<AsyncRepairResult>(this);
    connect(watcher, &QFutureWatcher<AsyncRepairResult>::finished, this, [this, watcher]() {
        auto repairRes = watcher->result();
        watcher->deleteLater();

        currentMesh_ = std::move(repairRes.mesh);
        const auto& result = repairRes.result;

        // RE-RENDER
        meshRenderer_->setMesh(currentMesh_);
        meshRenderer_->update();

        // UPDATE LABELS
        updateMeshInfo();

        // Build report message
        QString message;
        message += "=== MESH REPAIR REPORT ===\n\n";

        message += QString("Original triangles: %1\n").arg(result.originalTriangles);
        message += QString("Final triangles: %1\n").arg(result.finalTriangles);
        message += QString("Triangles removed: %1\n\n").arg(result.trianglesRemoved);

        message += QString("Vertices merged: %1\n").arg(result.verticesMerged);
        message += QString("Degenerate triangles removed: %1\n").arg(result.degenerateTrianglesRemoved);
        message += QString("Invalid triangles removed: %1\n\n").arg(result.invalidTrianglesRemoved);

        message += "Actions taken:\n";
        for (const auto& action : result.actions)
        {
            message += QString("  • %1\n").arg(QString::fromStdString(action));
        }

        statusBar()->showMessage(QString("Mesh repaired: %1 triangles removed, %2 vertices merged")
                                     .arg(result.trianglesRemoved)
                                     .arg(result.verticesMerged));

        setUiBusy(false);
        QMessageBox::information(this, "Mesh Repair Complete", message);

        qDebug() << "Repair complete:"
                 << result.trianglesRemoved << "triangles removed,"
                 << result.verticesMerged << "vertices merged";
    });

    auto meshCopy = currentMesh_;
    watcher->setFuture(QtConcurrent::run([meshCopy]() mutable -> AsyncRepairResult {
        AsyncRepairResult res;
        core::mesh::MeshRepairer repairer;
        res.result = repairer.repair(meshCopy);
        res.mesh = std::move(meshCopy);
        return res;
    }));
}

void MainWindow::updateMeshInfo()
{
    int triangles = currentMesh_.triangleCount();
    int vertices = triangles * 3;  // Approximate

    labelTriangleCount_->setText(QString("Triangles: %1").arg(triangles));
    labelVertexCount_->setText(QString("Vertices: ~%1").arg(vertices));
}

void MainWindow::onSliceMesh()
{
    qDebug() << "\n========================================";
    qDebug() << "🔪 SLICING STARTED";
    qDebug() << "========================================";

    if (currentMesh_.triangles.empty())
    {
        qDebug() << "❌ ERROR: No mesh loaded!";
        qDebug() << "========================================\n";
        return;
    }

    qDebug() << "📦 Mesh Info:";
    qDebug() << "   Triangles:" << currentMesh_.triangles.size();

    // Settings
    core::slicing::SlicingSettings settings;
    settings.layerHeight = 0.2f;
    settings.useSpatialIndex = true;

    qDebug() << "\n⚙️  Slicing Settings:";
    qDebug() << "   Layer Height:" << settings.layerHeight << "mm";
    qDebug() << "   Spatial Index:" << (settings.useSpatialIndex ? "✅ ON" : "❌ OFF");

    setUiBusy(true);
    statusBar()->showMessage("Slicing mesh...");

    qDebug() << "\n⏱️  Starting slicing...";

    auto* watcher = new QFutureWatcher<core::slicing::SlicingResult>(this);
    auto startTime = std::chrono::high_resolution_clock::now();

    connect(watcher, &QFutureWatcher<core::slicing::SlicingResult>::finished, this, [this, watcher, startTime]() {
        slicingResult_ = watcher->result();
        watcher->deleteLater();

        auto endTime = std::chrono::high_resolution_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                              endTime - startTime).count();

        // Sonuçlar
        qDebug() << "\n📊 RESULTS:";

        if (slicingResult_.success())
        {
            qDebug() << "   Status: ✅ SUCCESS";
            qDebug() << "   Layers:" << slicingResult_.layers.size();
            qDebug() << "   Segments:" << slicingResult_.totalSegments;
            qDebug() << "   Height:" << slicingResult_.totalHeight << "mm";
            qDebug() << "\n⏱️  PERFORMANCE:";
            qDebug() << "   Time:" << durationMs << "ms";
            qDebug() << "   Speed:" << (slicingResult_.layers.size() * 1000.0 / std::max(durationMs, 1LL)) << "layers/sec";

            // ⭐ UI UPDATE
            btnShowLayers_->setEnabled(true);
            sliderLayer_->setEnabled(true);
            sliderLayer_->setMaximum(slicingResult_.layers.size() - 1);
            sliderLayer_->setValue(0);

            labelLayerCount_->setText(QString("Layers: %1").arg(slicingResult_.layers.size()));

            statusBar()->showMessage(QString("✅ Slicing complete: %1 layers, %2 segments in %3 ms")
                                         .arg(slicingResult_.layers.size())
                                         .arg(slicingResult_.totalSegments)
                                         .arg(durationMs));

            setUiBusy(false);
            QMessageBox::information(this, "Slicing Complete",
                                     QString("✅ Slicing successful!\n\n"
                                             "Layers: %1\n"
                                             "Segments: %2\n"
                                             "Time: %3 ms\n"
                                             "Speed: %4 layers/sec\n\n"
                                             "Click 'Show Layers' to visualize.")
                                         .arg(slicingResult_.layers.size())
                                         .arg(slicingResult_.totalSegments)
                                         .arg(durationMs)
                                         .arg(slicingResult_.layers.size() * 1000.0 / std::max(durationMs, 1LL), 0, 'f', 1));
        }
        else
        {
            qDebug() << "   Status: ❌ FAILED";
            qDebug() << "   Error:" << QString::fromStdString(slicingResult_.errorMessage);

            statusBar()->showMessage("❌ Slicing failed!");
            setUiBusy(false);
            QMessageBox::critical(this, "Slicing Failed",
                                  QString("Slicing failed:\n\n%1")
                                      .arg(QString::fromStdString(slicingResult_.errorMessage)));
        }

        qDebug() << "========================================\n";
    });

    auto meshCopy = currentMesh_;
    watcher->setFuture(QtConcurrent::run([meshCopy, settings]() -> core::slicing::SlicingResult {
        core::slicing::Slicer slicer;
        return slicer.slice(meshCopy, settings);
    }));
}

void MainWindow::onShowLayers()
{
    if (slicingResult_.layers.empty())
    {
        QMessageBox::warning(this, "No Layers", "Please slice the mesh first.");
        return;
    }

    // Switch to layer rendering mode
    meshRenderer_->setRenderMode(rendering::MeshRenderer::RenderMode::Layers);
    meshRenderer_->setLayers(slicingResult_.layers);
    meshRenderer_->setCurrentLayer(-1);  // Show all layers

    statusBar()->showMessage("Showing all layers");
    qDebug() << "Showing" << slicingResult_.layers.size() << "layers";
}

void MainWindow::onLayerChanged(int value)
{
    if (slicingResult_.layers.empty())
    {
        return;
    }

    // Update current layer
    meshRenderer_->setCurrentLayer(value);

    // Update label
    labelCurrentLayer_->setText(QString("%1 / %2")
                                    .arg(value)
                                    .arg(slicingResult_.layers.size()));

    // Update status
    if (value >= 0 && value < static_cast<int>(slicingResult_.layers.size()))
    {
        const auto& layer = slicingResult_.layers[value];
        statusBar()->showMessage(QString("Layer %1: Z=%2mm, %3 segments")
                                     .arg(value)
                                     .arg(layer.zHeight(), 0, 'f', 2)
                                     .arg(layer.segmentCount()));
    }
}

void MainWindow::setUiBusy(bool busy)
{
    btnLoad_->setEnabled(!busy);
    btnSliceMesh_->setEnabled(!busy);
    btnRepairMesh_->setEnabled(!busy);
    btnRecalcNormals_->setEnabled(!busy);
    btnSmoothNormals_->setEnabled(!busy);
    btnFlipNormals_->setEnabled(!busy);
    btnWireframe_->setEnabled(!busy);
    btnSolid_->setEnabled(!busy);
    btnReset_->setEnabled(!busy);
}

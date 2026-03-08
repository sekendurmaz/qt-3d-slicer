#include "mainwindow.h"
#include "rendering/MeshRenderer.h"
#include "io/loading/CachedLoadingStrategy.h"
#include "io/loading/SyncLoadingStrategy.h"
#include "io/models/common/ModelFactory.h"
#include "core/mesh/MeshValidator.h"
#include "core/mesh/MeshAnalyzer.h"
#include "core/mesh/NormalProcessor.h"
#include "core/mesh/MeshRepairer.h"
#include "core/slicing/Slicer.h"
#include "ui/widgets/BuildPlatePanel.h"

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
#include <chrono>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Window setup
    setWindowTitle("Qt 3D Slicer");
    resize(1200, 800);

    // Central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main layout (HORIZONTAL - left panel + right 3D view)
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // LEFT PANEL - Build Plate Settings
    ui::widgets::BuildPlatePanel* platePanel = new ui::widgets::BuildPlatePanel(this);
    platePanel->setMaximumWidth(300);
    mainLayout->addWidget(platePanel);

    // RIGHT PANEL - 3D View + Controls
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(5);
    mainLayout->addWidget(rightPanel, 1);

    // OpenGL renderer widget
    meshRenderer_ = new rendering::MeshRenderer(this);
    meshRenderer_->setMinimumSize(800, 600);
    rightLayout->addWidget(meshRenderer_, 1);

    /* =========================== BUTTON LAYOUTS ===========================*/

    // First row: Main controls
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);

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

    QPushButton* btnCachedSync = new QPushButton("💾 Cached Sync", this);
    QPushButton* btnCachedAsync = new QPushButton("🚀💾 Cached Async", this);
    QPushButton* btnResetCamera = new QPushButton("📷 Reset Camera");
    btnCachedSync->setMinimumHeight(35);
    btnCachedAsync->setMinimumHeight(35);

    buttonLayout->addWidget(btnCachedSync);
    buttonLayout->addWidget(btnCachedAsync);
    buttonLayout->addWidget(btnResetCamera);
    buttonLayout->addStretch();

    rightLayout->addLayout(buttonLayout);

    // Second row: Normal Processing + Repair
    QHBoxLayout* buttonLayout2 = new QHBoxLayout();
    buttonLayout2->setSpacing(5);

    btnRecalcNormals_ = new QPushButton("Recalculate Normals", this);
    btnSmoothNormals_ = new QPushButton("Smooth Normals", this);
    btnFlipNormals_ = new QPushButton("Flip Normals", this);
    btnRepairMesh_ = new QPushButton("🔧 Repair Mesh", this);

    btnRecalcNormals_->setMinimumHeight(35);
    btnSmoothNormals_->setMinimumHeight(35);
    btnFlipNormals_->setMinimumHeight(35);
    btnRepairMesh_->setMinimumHeight(35);

    buttonLayout2->addWidget(btnRecalcNormals_);
    buttonLayout2->addWidget(btnSmoothNormals_);
    buttonLayout2->addWidget(btnFlipNormals_);
    buttonLayout2->addWidget(btnRepairMesh_);
    buttonLayout2->addStretch();

    rightLayout->addLayout(buttonLayout2);

    // Third row: Slicing
    QHBoxLayout* buttonLayout3 = new QHBoxLayout();
    buttonLayout3->setSpacing(5);

    btnSliceMesh_ = new QPushButton("🍕 Slice Mesh", this);
    btnSliceMesh_->setMinimumHeight(35);
    btnSliceMesh_->setStyleSheet("QPushButton { font-weight: bold; background-color: #4CAF50; color: white; }");

    btnShowLayers_ = new QPushButton("👁️ Show Layers", this);
    btnShowLayers_->setMinimumHeight(35);
    btnShowLayers_->setStyleSheet("QPushButton { font-weight: bold; background-color: #2196F3; color: white; }");
    btnShowLayers_->setEnabled(false);

    btnExportLayers_ = new QPushButton("💾 Export Layers", this);
    btnExportLayers_->setMinimumHeight(35);
    btnExportLayers_->setStyleSheet("QPushButton { font-weight: bold; background-color: #FF9800; color: white; }");
    btnExportLayers_->setEnabled(false);

    buttonLayout3->addWidget(btnSliceMesh_);
    buttonLayout3->addWidget(btnShowLayers_);
    buttonLayout3->addWidget(btnExportLayers_);
    buttonLayout3->addStretch();

    rightLayout->addLayout(buttonLayout3);

    // ===== TRANSFORM CONTROLS ===== ← 6-DOF VERSION!
    QVBoxLayout* transformBox = new QVBoxLayout();

    // ROW 1: Move X, Y, Z
    QHBoxLayout* moveLayout = new QHBoxLayout();

    QLabel* labelMoveX = new QLabel("Move X:", this);
    spinMoveX_ = new QDoubleSpinBox(this);
    spinMoveX_->setRange(-500.0, 500.0);
    spinMoveX_->setSingleStep(1.0);
    spinMoveX_->setValue(0.0);
    spinMoveX_->setMinimumWidth(70);
    spinMoveX_->setSuffix(" mm");

    QLabel* labelMoveY = new QLabel("Y:", this);
    spinMoveY_ = new QDoubleSpinBox(this);
    spinMoveY_->setRange(-500.0, 500.0);
    spinMoveY_->setSingleStep(1.0);
    spinMoveY_->setValue(0.0);
    spinMoveY_->setMinimumWidth(70);
    spinMoveY_->setSuffix(" mm");

    QLabel* labelMoveZ = new QLabel("Z:", this);
    spinMoveZ_ = new QDoubleSpinBox(this);
    spinMoveZ_->setRange(0.0, 500.0);
    spinMoveZ_->setSingleStep(1.0);
    spinMoveZ_->setValue(0.0);
    spinMoveZ_->setMinimumWidth(70);
    spinMoveZ_->setSuffix(" mm");

    moveLayout->addWidget(labelMoveX);
    moveLayout->addWidget(spinMoveX_);
    moveLayout->addWidget(labelMoveY);
    moveLayout->addWidget(spinMoveY_);
    moveLayout->addWidget(labelMoveZ);
    moveLayout->addWidget(spinMoveZ_);
    moveLayout->addStretch();

    transformBox->addLayout(moveLayout);

    // ROW 2: Rotate X, Y, Z
    QHBoxLayout* rotateLayout = new QHBoxLayout();

    QLabel* labelRotateX = new QLabel("Rotate X:", this);
    spinRotateX_ = new QDoubleSpinBox(this);
    spinRotateX_->setRange(-180.0, 180.0);
    spinRotateX_->setSingleStep(5.0);
    spinRotateX_->setValue(0.0);
    spinRotateX_->setMinimumWidth(70);
    spinRotateX_->setSuffix("°");

    QLabel* labelRotateY = new QLabel("Y:", this);
    spinRotateY_ = new QDoubleSpinBox(this);
    spinRotateY_->setRange(-180.0, 180.0);
    spinRotateY_->setSingleStep(5.0);
    spinRotateY_->setValue(0.0);
    spinRotateY_->setMinimumWidth(70);
    spinRotateY_->setSuffix("°");

    QLabel* labelRotateZ = new QLabel("Z:", this);
    spinRotateZ_ = new QDoubleSpinBox(this);
    spinRotateZ_->setRange(-180.0, 180.0);
    spinRotateZ_->setSingleStep(5.0);
    spinRotateZ_->setValue(0.0);
    spinRotateZ_->setMinimumWidth(70);
    spinRotateZ_->setSuffix("°");

    rotateLayout->addWidget(labelRotateX);
    rotateLayout->addWidget(spinRotateX_);
    rotateLayout->addWidget(labelRotateY);
    rotateLayout->addWidget(spinRotateY_);
    rotateLayout->addWidget(labelRotateZ);
    rotateLayout->addWidget(spinRotateZ_);
    rotateLayout->addStretch();

    transformBox->addLayout(rotateLayout);

    // ROW 3: Buttons
    QHBoxLayout* transformButtons = new QHBoxLayout();
    btnCenterModel_ = new QPushButton("📍 Center", this);
    btnResetTransform_ = new QPushButton("🔄 Reset", this);

    transformButtons->addWidget(btnCenterModel_);
    transformButtons->addWidget(btnResetTransform_);
    transformButtons->addStretch();

    transformBox->addLayout(transformButtons);

    rightLayout->addLayout(transformBox);

    // Fourth row: Layer slider
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
    sliderLayout->addWidget(sliderLayer_, 1);
    sliderLayout->addWidget(labelCurrentLayer_);

    rightLayout->addLayout(sliderLayout);

    /* =========================== STATUS BAR WITH LABELS ===========================*/

    labelTriangleCount_ = new QLabel("Triangles: 0", this);
    labelTriangleCount_->setStyleSheet("QLabel { padding: 5px; font-weight: bold; }");
    statusBar()->addPermanentWidget(labelTriangleCount_);

    labelVertexCount_ = new QLabel("Vertices: 0", this);
    labelVertexCount_->setStyleSheet("QLabel { padding: 5px; }");
    statusBar()->addPermanentWidget(labelVertexCount_);

    labelLayerCount_ = new QLabel("Layers: 0", this);
    labelLayerCount_->setStyleSheet("QLabel { padding: 5px; color: #4CAF50; font-weight: bold; }");
    statusBar()->addPermanentWidget(labelLayerCount_);

    statusBar()->showMessage("Ready - Load a 3D model to begin");

    // ===== CONNECT SIGNALS =====
    connect(btnLoad_, &QPushButton::clicked, this, &MainWindow::onLoadModel);
    connect(btnWireframe_, &QPushButton::clicked, this, &MainWindow::onWireframe);
    connect(btnSolid_, &QPushButton::clicked, this, &MainWindow::onSolid);
    connect(btnReset_, &QPushButton::clicked, this, &MainWindow::onResetView);

    connect(btnCachedSync, &QPushButton::clicked, this, &MainWindow::onLoadModelCachedSync);
    connect(btnCachedAsync, &QPushButton::clicked, this, &MainWindow::onLoadModelCachedAsync);
    connect(btnResetCamera, &QPushButton::clicked, [this]() {
        meshRenderer_->resetCamera();
    });

    connect(btnRecalcNormals_, &QPushButton::clicked, this, &MainWindow::onRecalculateNormals);
    connect(btnSmoothNormals_, &QPushButton::clicked, this, &MainWindow::onSmoothNormals);
    connect(btnFlipNormals_, &QPushButton::clicked, this, &MainWindow::onFlipNormals);

    connect(btnRepairMesh_, &QPushButton::clicked, this, &MainWindow::onRepairMesh);

    connect(btnSliceMesh_, &QPushButton::clicked, this, &MainWindow::onSliceMesh);
    connect(btnShowLayers_, &QPushButton::clicked, this, &MainWindow::onShowLayers);
    connect(btnExportLayers_, &QPushButton::clicked, this, &MainWindow::onExportLayers);
    connect(sliderLayer_, &QSlider::valueChanged, this, &MainWindow::onLayerChanged);

    // Transform signals - 6-DOF!
    connect(spinMoveX_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onMoveXChanged);
    connect(spinMoveY_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onMoveYChanged);
    connect(spinMoveZ_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onMoveZChanged);

    connect(spinRotateX_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onRotateXChanged);
    connect(spinRotateY_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onRotateYChanged);
    connect(spinRotateZ_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onRotateZChanged);

    connect(btnCenterModel_, &QPushButton::clicked, this, &MainWindow::onCenterModel);
    connect(btnResetTransform_, &QPushButton::clicked, this, &MainWindow::onResetTransform);

    // Gizmo signal
    connect(meshRenderer_, &rendering::MeshRenderer::modelTransformed,
            this, &MainWindow::onModelTransformedByGizmo);

    // Initialize loading strategies
    m_loadingStrategy = std::make_unique<io::loading::SyncLoadingStrategy>();
    m_cachedSyncStrategy = std::make_unique<io::loading::CachedLoadingStrategy>(
        std::make_unique<io::loading::SyncLoadingStrategy>()
        );
    m_cachedAsyncStrategy = std::make_unique<io::loading::CachedLoadingStrategy>(
        std::make_unique<io::loading::AsyncLoadingStrategy>()
        );

    connect(platePanel, &ui::widgets::BuildPlatePanel::plateCreated,
            this, &MainWindow::onPlateCreated);
}

MainWindow::~MainWindow()
{
}

// ... (onLoadModel, onWireframe, onSolid, etc. - HİÇ DEĞİŞMEDİ)

void MainWindow::onMoveXChanged(double value)
{
    QVector3D pos = meshRenderer_->getModelTranslation();
    meshRenderer_->setModelTranslation(value, pos.y(), pos.z());  // ← z eklendi!
}

void MainWindow::onMoveYChanged(double value)
{
    QVector3D pos = meshRenderer_->getModelTranslation();
    meshRenderer_->setModelTranslation(pos.x(), value, pos.z());  // ← z eklendi!
}

void MainWindow::onMoveZChanged(double value)  // ← YENİ!
{
    QVector3D pos = meshRenderer_->getModelTranslation();
    meshRenderer_->setModelTranslation(pos.x(), pos.y(), value);
}

void MainWindow::onRotateXChanged(double value)  // ← YENİ!
{
    QVector3D rot = meshRenderer_->getModelRotation();
    meshRenderer_->setModelRotation(value, rot.y(), rot.z());
}

void MainWindow::onRotateYChanged(double value)  // ← YENİ!
{
    QVector3D rot = meshRenderer_->getModelRotation();
    meshRenderer_->setModelRotation(rot.x(), value, rot.z());
}

void MainWindow::onRotateZChanged(double value)
{
    QVector3D rot = meshRenderer_->getModelRotation();  // ← DEĞİŞTİ!
    meshRenderer_->setModelRotation(rot.x(), rot.y(), value);  // ← DEĞİŞTİ!
}

void MainWindow::onResetTransform()
{
    meshRenderer_->resetModelTransform();

    spinMoveX_->blockSignals(true);
    spinMoveY_->blockSignals(true);
    spinMoveZ_->blockSignals(true);     // ← YENİ!
    spinRotateX_->blockSignals(true);   // ← YENİ!
    spinRotateY_->blockSignals(true);   // ← YENİ!
    spinRotateZ_->blockSignals(true);

    spinMoveX_->setValue(0.0);
    spinMoveY_->setValue(0.0);
    spinMoveZ_->setValue(0.0);    // ← YENİ!
    spinRotateX_->setValue(0.0);  // ← YENİ!
    spinRotateY_->setValue(0.0);  // ← YENİ!
    spinRotateZ_->setValue(0.0);

    spinMoveX_->blockSignals(false);
    spinMoveY_->blockSignals(false);
    spinMoveZ_->blockSignals(false);     // ← YENİ!
    spinRotateX_->blockSignals(false);   // ← YENİ!
    spinRotateY_->blockSignals(false);   // ← YENİ!
    spinRotateZ_->blockSignals(false);
}

void MainWindow::onModelTransformedByGizmo(QVector3D translation, QVector3D rotation)  // ← PARAMETRE DEĞİŞTİ!
{
    if (spinMoveX_) {
        spinMoveX_->blockSignals(true);
        spinMoveX_->setValue(translation.x());
        spinMoveX_->blockSignals(false);
    }

    if (spinMoveY_) {
        spinMoveY_->blockSignals(true);
        spinMoveY_->setValue(translation.y());
        spinMoveY_->blockSignals(false);
    }

    if (spinMoveZ_) {
        spinMoveZ_->blockSignals(true);
        spinMoveZ_->setValue(translation.z());
        spinMoveZ_->blockSignals(false);
    }

    if (spinRotateX_) {
        spinRotateX_->blockSignals(true);
        spinRotateX_->setValue(rotation.x());
        spinRotateX_->blockSignals(false);
    }

    if (spinRotateY_) {
        spinRotateY_->blockSignals(true);
        spinRotateY_->setValue(rotation.y());
        spinRotateY_->blockSignals(false);
    }

    if (spinRotateZ_) {
        spinRotateZ_->blockSignals(true);
        spinRotateZ_->setValue(rotation.z());
        spinRotateZ_->blockSignals(false);
    }
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
    statusBar()->showMessage("Loading model...");

    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        currentMesh_ = io::models::ModelFactory::loadModel(fileName.toStdString());

        auto endTime = std::chrono::high_resolution_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                              endTime - startTime
                              ).count();

        qDebug() << "⏱️  Load time:" << durationMs << "ms";

        meshRenderer_->setMesh(currentMesh_);
        updateMeshInfo();

        core::mesh::MeshValidator validator;
        auto validResult = validator.validate(currentMesh_);

        core::mesh::MeshAnalyzer analyzer;
        auto stats = analyzer.analyze(currentMesh_);

        QString statusMsg = QString("Loaded: %1 - %2 triangles, Volume: %3 mm³")
                                .arg(QFileInfo(fileName).fileName())
                                .arg(stats.triangleCount)
                                .arg(stats.volume, 0, 'f', 2);
        statusBar()->showMessage(statusMsg);

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

        QMessageBox::information(this, "Model Analysis", message);

        qDebug() << "Rendered:" << stats.triangleCount << "triangles";

    } catch (const std::exception& e) {
        statusBar()->showMessage("Error loading model");
        QMessageBox::critical(this, "Error", QString("Failed to load model:\n\n%1").arg(e.what()));
        qDebug() << "Error:" << e.what();
    }
}

void MainWindow::onLoadModelAsync()
{
    QMessageBox::information(this, "Info", "Async loading not fully implemented yet. Use cached async!");
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

    core::mesh::NormalProcessor processor;
    auto result = processor.recalculateNormals(currentMesh_);

    meshRenderer_->setMesh(currentMesh_);
    updateMeshInfo();

    QString msg = QString("Recalculated %1 normals").arg(result.normalsRecalculated);
    statusBar()->showMessage(msg);
    QMessageBox::information(this, "Normals Recalculated", msg);
}

void MainWindow::onSmoothNormals()
{
    if (currentMesh_.triangleCount() == 0)
    {
        QMessageBox::warning(this, "No Mesh", "Please load a model first.");
        return;
    }

    core::mesh::NormalProcessor processor;
    auto result = processor.smoothNormals(currentMesh_, 30.0f);

    meshRenderer_->setMesh(currentMesh_);
    updateMeshInfo();

    QString msg = QString("Smoothed %1 normals (angle threshold: 30°)")
                      .arg(result.normalsSmoothed);
    statusBar()->showMessage(msg);
    QMessageBox::information(this, "Normals Smoothed", msg);
}

void MainWindow::onFlipNormals()
{
    if (currentMesh_.triangleCount() == 0)
    {
        QMessageBox::warning(this, "No Mesh", "Please load a model first.");
        return;
    }

    core::mesh::NormalProcessor processor;
    auto result = processor.flipNormals(currentMesh_);

    meshRenderer_->setMesh(currentMesh_);
    updateMeshInfo();

    QString msg = QString("Flipped %1 normals").arg(result.normalsFlipped);
    statusBar()->showMessage(msg);
    QMessageBox::information(this, "Normals Flipped", msg);
}

void MainWindow::onRepairMesh()
{
    if (currentMesh_.triangleCount() == 0)
    {
        QMessageBox::warning(this, "No Mesh", "Please load a model first.");
        return;
    }

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

    statusBar()->showMessage("Repairing mesh...");

    core::mesh::MeshRepairer repairer;
    auto result = repairer.repair(currentMesh_);

    meshRenderer_->setMesh(currentMesh_);
    meshRenderer_->update();
    updateMeshInfo();

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

    QMessageBox::information(this, "Mesh Repair Complete", message);

    qDebug() << "Repair complete:"
             << result.trianglesRemoved << "triangles removed,"
             << result.verticesMerged << "vertices merged";
}

void MainWindow::updateMeshInfo()
{
    int triangles = currentMesh_.triangleCount();
    int vertices = triangles * 3;

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

    core::slicing::SlicingSettings settings;
    settings.layerHeight = 0.03f;
    settings.useSpatialIndex = true;

    qDebug() << "\n⚙️  Slicing Settings:";
    qDebug() << "   Layer Height:" << settings.layerHeight << "mm";
    qDebug() << "   Spatial Index:" << (settings.useSpatialIndex ? "✅ ON" : "❌ OFF");

    qDebug() << "\n⏱️  Starting slicing...";
    auto startTime = std::chrono::high_resolution_clock::now();

    core::slicing::Slicer slicer;
    slicingResult_ = slicer.slice(currentMesh_, settings);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                          endTime - startTime
                          ).count();

    qDebug() << "\n📊 RESULTS:";

    if (slicingResult_.success())
    {
        qDebug() << "   Status: ✅ SUCCESS";
        qDebug() << "   Layers:" << slicingResult_.layers.size();
        qDebug() << "   Segments:" << slicingResult_.totalSegments;
        qDebug() << "   Height:" << slicingResult_.totalHeight << "mm";
        qDebug() << "\n⏱️  PERFORMANCE:";
        qDebug() << "   Time:" << durationMs << "ms";
        qDebug() << "   Speed:" << (slicingResult_.layers.size() * 1000.0 / durationMs) << "layers/sec";

        double totalOps = static_cast<double>(currentMesh_.triangles.size()) * slicingResult_.layers.size();
        double opsPerSec = totalOps / (durationMs / 1000.0);
        qDebug() << "   Throughput:" << static_cast<long long>(opsPerSec) << "triangle-checks/sec";

        btnShowLayers_->setEnabled(true);
        btnExportLayers_->setEnabled(true);
        sliderLayer_->setEnabled(true);
        sliderLayer_->setMaximum(slicingResult_.layers.size() - 1);
        sliderLayer_->setValue(0);

        labelLayerCount_->setText(QString("Layers: %1").arg(slicingResult_.layers.size()));

        statusBar()->showMessage(QString("✅ Slicing complete: %1 layers, %2 segments in %3 ms")
                                     .arg(slicingResult_.layers.size())
                                     .arg(slicingResult_.totalSegments)
                                     .arg(durationMs));

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
                                     .arg(slicingResult_.layers.size() * 1000.0 / durationMs, 0, 'f', 1));
    }
    else
    {
        qDebug() << "   Status: ❌ FAILED";
        qDebug() << "   Error:" << QString::fromStdString(slicingResult_.errorMessage);

        statusBar()->showMessage("❌ Slicing failed!");
        QMessageBox::critical(this, "Slicing Failed",
                              QString("Slicing failed:\n\n%1")
                                  .arg(QString::fromStdString(slicingResult_.errorMessage)));
    }

    qDebug() << "========================================\n";
}

void MainWindow::onShowLayers()
{
    if (slicingResult_.layers.empty())
    {
        QMessageBox::warning(this, "No Layers", "Please slice the mesh first.");
        return;
    }

    meshRenderer_->setRenderMode(rendering::MeshRenderer::RenderMode::Layers);
    meshRenderer_->setLayers(slicingResult_.layers);
    meshRenderer_->setCurrentLayer(-1);

    statusBar()->showMessage("Showing all layers");
    qDebug() << "Showing" << slicingResult_.layers.size() << "layers";
}

void MainWindow::onLayerChanged(int value)
{
    if (slicingResult_.layers.empty())
    {
        return;
    }

    meshRenderer_->setCurrentLayer(value);

    labelCurrentLayer_->setText(QString("%1 / %2")
                                    .arg(value)
                                    .arg(slicingResult_.layers.size()));

    if (value >= 0 && value < static_cast<int>(slicingResult_.layers.size()))
    {
        const auto& layer = slicingResult_.layers[value];
        statusBar()->showMessage(QString("Layer %1: Z=%2mm, %3 segments")
                                     .arg(value)
                                     .arg(layer.zHeight(), 0, 'f', 2)
                                     .arg(layer.segmentCount()));
    }
}

void MainWindow::onLoadModelCachedSync()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select 3D Model (Cached Sync)",
        "",
        "3D Models (*.stl *.obj *.3mf);;STL Files (*.stl);;OBJ Files (*.obj);;3MF Files (*.3mf);;All Files (*)"
        );

    if (fileName.isEmpty()) {
        return;
    }

    qDebug() << "💾 CACHED SYNC Loading:" << fileName;
    statusBar()->showMessage("Loading model (cached)...");

    auto startTime = std::chrono::high_resolution_clock::now();

    m_cachedSyncStrategy->load(
        fileName.toStdString(),

        [](int progress) {
            qDebug() << "Progress:" << progress << "%";
        },

        [this, startTime, fileName](core::mesh::Mesh mesh, bool success, std::string error) {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto loadMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                              endTime - startTime
                              ).count();

            if (!success) {
                qDebug() << "❌ Cached sync load failed:" << QString::fromStdString(error);
                statusBar()->showMessage("❌ Load failed");
                QMessageBox::critical(this, "Error", QString("Failed to load:\n\n%1").arg(QString::fromStdString(error)));
                return;
            }

            qDebug() << "✅ Load time:" << loadMs << "ms";

            currentMesh_ = std::move(mesh);
            meshRenderer_->setMesh(currentMesh_);
            updateMeshInfo();

            core::mesh::MeshAnalyzer analyzer;
            auto stats = analyzer.analyze(currentMesh_);

            QString statusMsg = QString("Loaded: %1 - %2 triangles in %3ms (cached)")
                                    .arg(QFileInfo(fileName).fileName())
                                    .arg(stats.triangleCount)
                                    .arg(loadMs);
            statusBar()->showMessage(statusMsg);

            qDebug() << "Rendered:" << stats.triangleCount << "triangles";
        }
        );
}

void MainWindow::onLoadModelCachedAsync()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select 3D Model (Cached Async)",
        "",
        "3D Models (*.stl *.obj *.3mf);;STL Files (*.stl);;OBJ Files (*.obj);;3MF Files (*.3mf);;All Files (*)"
        );

    if (fileName.isEmpty()) {
        return;
    }

    qDebug() << "🚀💾 CACHED ASYNC Loading:" << fileName;
    statusBar()->showMessage("Loading model (cached async)...");

    auto startTime = std::chrono::high_resolution_clock::now();

    m_cachedAsyncStrategy->load(
        fileName.toStdString(),

        [](int progress) {
            qDebug() << "📊 Progress:" << progress << "%";
        },

        [this, startTime, fileName](core::mesh::Mesh mesh, bool success, std::string error) {
            auto loadMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::high_resolution_clock::now() - startTime
                              ).count();

            if (!success) {
                qDebug() << "❌ Cached async load failed:" << QString::fromStdString(error);
                QMetaObject::invokeMethod(this, [this, error]() {
                    statusBar()->showMessage("❌ Load failed");
                    QMessageBox::critical(this, "Error", QString::fromStdString(error));
                }, Qt::QueuedConnection);
                return;
            }

            qDebug() << "✅ CACHED ASYNC Load completed in" << loadMs << "ms";

            QMetaObject::invokeMethod(this, [this, mesh = std::move(mesh), loadMs, fileName]() mutable {
                currentMesh_ = std::move(mesh);
                meshRenderer_->setMesh(currentMesh_);
                updateMeshInfo();

                core::mesh::MeshAnalyzer analyzer;
                auto stats = analyzer.analyze(currentMesh_);

                statusBar()->showMessage(QString("✅ Loaded %1 triangles in %2ms (cached async)")
                                             .arg(stats.triangleCount)
                                             .arg(loadMs));

                qDebug() << "✅ Total complete:" << currentMesh_.triangleCount() << "triangles";
            }, Qt::QueuedConnection);
        }
        );
}

void MainWindow::onPlateCreated(std::shared_ptr<core::buildplate::BuildPlate> plate)
{
    currentPlate_ = plate;

    qDebug() << "✅ Build plate created:";
    qDebug() << "   Type:" << (plate->type() == core::buildplate::PlateType::Rectangular ? "Rectangular" : "Circular");
    qDebug() << "   Width:" << plate->width() << "mm";
    qDebug() << "   Depth:" << plate->depth() << "mm";
    qDebug() << "   Height:" << plate->height() << "mm";

    meshRenderer_->setBuildPlate(plate);

    statusBar()->showMessage(QString("Build plate created: %1×%2×%3mm")
                                 .arg(plate->width(), 0, 'f', 0)
                                 .arg(plate->depth(), 0, 'f', 0)
                                 .arg(plate->height(), 0, 'f', 1));
}

void MainWindow::onExportLayers()
{
    if (slicingResult_.layers.empty())
    {
        QMessageBox::warning(this, "No Layers", "Please slice the mesh first.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Layer Data",
        "layers.slyrz",
        "Compressed Binary (*.slyrz);;"
        "Binary Layers (*.slyr);;"
        "JSON Files (*.json);;"
        "All Files (*)"
        );

    if (fileName.isEmpty()) return;

    statusBar()->showMessage("Exporting...");
    auto start = std::chrono::high_resolution_clock::now();

    bool success = false;
    QString ext = QFileInfo(fileName).suffix().toLower();

    if (ext == "slyrz")
    {
        success = exportLayersBinaryCompressed(fileName);
    }
    else if (ext == "slyr")
    {
        success = exportLayersBinary(fileName);
    }
    else
    {
        success = exportLayersJSON(fileName);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    if (success)
    {
        qint64 size = QFileInfo(fileName).size();
        QString sizeStr;

        if (size > 1024 * 1024)
        {
            sizeStr = QString::number(size / 1024.0 / 1024.0, 'f', 2) + " MB";
        }
        else
        {
            sizeStr = QString::number(size / 1024.0, 'f', 2) + " KB";
        }

        statusBar()->showMessage(QString("✅ Exported in %1ms (%2)").arg(ms).arg(sizeStr));

        QMessageBox::information(this, "Export Complete",
                                 QString("Exported %1 layers\nSize: %2\nTime: %3 ms")
                                     .arg(slicingResult_.layers.size())
                                     .arg(sizeStr)
                                     .arg(ms));
    }
    else
    {
        QMessageBox::critical(this, "Export Failed", "Failed to write file.");
    }
}

bool MainWindow::exportLayersJSON(const QString& fileName)
{
    QJsonObject root;

    root["total_layers"] = static_cast<int>(slicingResult_.layers.size());
    root["total_segments"] = slicingResult_.totalSegments;
    root["total_height"] = slicingResult_.totalHeight;
    root["layer_height"] = 0.03f;

    QJsonObject layersObject;

    for (size_t i = 0; i < slicingResult_.layers.size(); ++i)
    {
        const auto& layer = slicingResult_.layers[i];
        QString zKey = QString::number(layer.zHeight(), 'f', 2);
        QJsonArray segmentsArray;

        for (const auto& segment : layer.segments())
        {
            QJsonArray seg;
            seg.append(segment.start.x);
            seg.append(segment.start.y);
            seg.append(segment.end.x);
            seg.append(segment.end.y);
            segmentsArray.append(seg);
        }

        layersObject[zKey] = segmentsArray;
    }

    root["layers"] = layersObject;

    QJsonDocument doc(root);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    file.write(jsonData);
    file.close();

    qint64 fileSize = QFileInfo(fileName).size();
    qDebug() << "✅ Exported" << slicingResult_.layers.size() << "layers";
    qDebug() << "   File size:" << (fileSize / 1024.0 / 1024.0) << "MB";

    return true;
}

bool MainWindow::exportLayersBinary(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);

    out.writeRawData("SLYR", 4);
    out << (quint32)1;
    out << (float)0.03f;
    out << (quint32)slicingResult_.layers.size();
    out << (quint32)slicingResult_.totalSegments;
    out << (float)slicingResult_.totalHeight;

    for (const auto& layer : slicingResult_.layers)
    {
        out << (float)layer.zHeight();
        out << (quint32)layer.segmentCount();

        for (const auto& segment : layer.segments())
        {
            out << (float)segment.start.x;
            out << (float)segment.start.y;
            out << (float)segment.end.x;
            out << (float)segment.end.y;
        }
    }

    file.close();

    qint64 fileSize = QFileInfo(fileName).size();
    qDebug() << "✅ Binary export:" << (fileSize / 1024.0 / 1024.0) << "MB";

    return true;
}

bool MainWindow::exportLayersBinaryCompressed(const QString& fileName)
{
    QByteArray binaryData;
    QDataStream out(&binaryData, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);

    out.writeRawData("SLYR", 4);
    out << (quint32)1;
    out << (float)0.03f;
    out << (quint32)slicingResult_.layers.size();
    out << (quint32)slicingResult_.totalSegments;
    out << (float)slicingResult_.totalHeight;

    for (const auto& layer : slicingResult_.layers)
    {
        out << (float)layer.zHeight();
        out << (quint32)layer.segmentCount();

        for (const auto& segment : layer.segments())
        {
            out << (float)segment.start.x;
            out << (float)segment.start.y;
            out << (float)segment.end.x;
            out << (float)segment.end.y;
        }
    }

    qDebug() << "📊 Uncompressed:" << (binaryData.size() / 1024.0 / 1024.0) << "MB";

    QByteArray compressed = qCompress(binaryData, 9);

    qDebug() << "📦 Compressed:" << (compressed.size() / 1024.0 / 1024.0) << "MB";
    qDebug() << "✅ Ratio:" << (100.0 * compressed.size() / binaryData.size()) << "%";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    file.write(compressed);
    file.close();

    return true;
}

void MainWindow::onCenterModel()
{
    if (currentMesh_.triangles.empty())
    {
        qDebug() << "⚠️ No mesh to center!";
        return;
    }

    meshRenderer_->centerModel(currentMesh_);

    QVector3D pos = meshRenderer_->getModelTranslation();

    spinMoveX_->blockSignals(true);
    spinMoveY_->blockSignals(true);
    spinMoveZ_->blockSignals(true);

    spinMoveX_->setValue(pos.x());
    spinMoveY_->setValue(pos.y());
    spinMoveZ_->setValue(pos.z());

    spinMoveX_->blockSignals(false);
    spinMoveY_->blockSignals(false);
    spinMoveZ_->blockSignals(false);
}

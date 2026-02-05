#include "mainwindow.h"
#include "rendering/MeshRenderer.h"
#include "io/models/common/ModelFactory.h"
#include "core/mesh/MeshValidator.h"
#include "core/mesh/MeshAnalyzer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QFileInfo>
#include <QDebug>

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

    // Button layout (horizontal)
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

    // Status bar
    statusBar()->showMessage("Ready - Load a 3D model to begin");

    // Connect signals
    connect(btnLoad_, &QPushButton::clicked, this, &MainWindow::onLoadModel);
    connect(btnWireframe_, &QPushButton::clicked, this, &MainWindow::onWireframe);
    connect(btnSolid_, &QPushButton::clicked, this, &MainWindow::onSolid);
    connect(btnReset_, &QPushButton::clicked, this, &MainWindow::onResetView);
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
    statusBar()->showMessage("Loading model...");

    try {
        // Load mesh
        currentMesh_ = io::models::ModelFactory::loadModel(fileName.toStdString());

        // Render mesh
        meshRenderer_->setMesh(currentMesh_);

        // Validate
        core::mesh::MeshValidator validator;
        auto validResult = validator.validate(currentMesh_);

        // Analyze
        core::mesh::MeshAnalyzer analyzer;
        auto stats = analyzer.analyze(currentMesh_);

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

        QMessageBox::information(this, "Model Analysis", message);

        qDebug() << "Rendered:" << stats.triangleCount << "triangles";

    } catch (const std::exception& e) {
        statusBar()->showMessage("Error loading model");
        QMessageBox::critical(this, "Error", QString("Failed to load model:\n\n%1").arg(e.what()));
        qDebug() << "Error:" << e.what();
    }
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

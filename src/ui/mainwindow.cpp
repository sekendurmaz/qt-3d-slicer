#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>

#include "io/models/common/ModelFactory.h"
#include "core/mesh/MeshValidator.h"
#include "core/mesh/MeshAnalyzer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::clicked,
            this, &MainWindow::onPushButtonClicked);
    setWindowTitle("3D Model Reader");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onPushButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "3D Model Dosyasi Sec",
        "",
        "3D Models (*.stl *.obj *.3mf);;All Files (*)"
        );

    if (fileName.isEmpty()) {
        return;
    }

    qDebug() << "Secilen dosya:" << fileName;

    try {
        auto mesh = io::models::ModelFactory::loadModel(fileName.toStdString());

        // VALIDATION
        core::mesh::MeshValidator validator;
        auto validResult = validator.validate(mesh);

        // ANALYSIS
        core::mesh::MeshAnalyzer analyzer;
        auto stats = analyzer.analyze(mesh);

        // Mesaj oluştur
        QString message;

        message += "=== MODEL STATISTICS ===\n\n";
        message += QString("File: %1\n\n").arg(QFileInfo(fileName).fileName());

        // Temel bilgiler
        message += QString("Triangles: %1\n").arg(stats.triangleCount);
        message += QString("Vertices: ~%1\n\n").arg(stats.vertexCount);

        // Bounding Box
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

        // Geometrik özellikler
        message += "=== GEOMETRY ===\n";
        message += QString("Surface Area: %1 mm²\n")
                       .arg(stats.surfaceArea, 0, 'f', 2);
        message += QString("Volume: %1 mm³\n")
                       .arg(stats.volume, 0, 'f', 2);
        message += QString("Watertight: %1\n\n")
                       .arg(stats.isWatertight ? "Yes" : "No");

        // Center
        message += QString("Center: (%1, %2, %3)\n\n")
                       .arg(stats.centerOfMass.x, 0, 'f', 2)
                       .arg(stats.centerOfMass.y, 0, 'f', 2)
                       .arg(stats.centerOfMass.z, 0, 'f', 2);

        // Validation
        message += "=== VALIDATION ===\n";
        if (validResult.isValid) {
            message += "Status: VALID\n";
        } else {
            message += "Status: HAS ERRORS\n";
        }

        message += QString("Degenerate triangles: %1\n").arg(validResult.degenerateTriangles);
        message += QString("Invalid vertices: %1\n").arg(validResult.invalidVertices);
        message += QString("Duplicate vertices: %1").arg(validResult.duplicateVertices);

        QMessageBox::information(this, "Model Analysis", message);

        qDebug() << "Analysis complete:"
                 << stats.triangleCount << "triangles,"
                 << stats.volume << "mm³";

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Hata", QString::fromStdString(e.what()));
        qDebug() << "Hata:" << e.what();
    }
}

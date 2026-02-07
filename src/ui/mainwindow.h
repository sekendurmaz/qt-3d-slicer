#pragma once

#include <QMainWindow>
#include "core/mesh/mesh.h"
#include "core/slicing/Slicer.h"  // ← SlicingResult için

// Forward declarations
namespace rendering {
class MeshRenderer;
}

class QPushButton;
class QLabel;
class QSlider;  // ← YENİ!

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadModel();
    void onWireframe();
    void onSolid();
    void onResetView();

    // Normal Processing
    void onRecalculateNormals();
    void onSmoothNormals();
    void onFlipNormals();

    // Mesh Repair
    void onRepairMesh();

    // Slicing
    void onSliceMesh();
    void onShowLayers();        // ← YENİ!
    void onLayerChanged(int);   // ← YENİ! (slider)

private:
    // UI components
    rendering::MeshRenderer* meshRenderer_;
    QPushButton* btnLoad_;
    QPushButton* btnWireframe_;
    QPushButton* btnSolid_;
    QPushButton* btnReset_;

    // Normal buttons
    QPushButton* btnRecalcNormals_;
    QPushButton* btnSmoothNormals_;
    QPushButton* btnFlipNormals_;

    // Repair button
    QPushButton* btnRepairMesh_;

    // Slice buttons
    QPushButton* btnSliceMesh_;
    QPushButton* btnShowLayers_;  // ← YENİ!

    // Layer slider ← YENİ!
    QSlider* sliderLayer_;
    QLabel* labelCurrentLayer_;

    // Status labels
    QLabel* labelTriangleCount_;
    QLabel* labelVertexCount_;
    QLabel* labelLayerCount_;

    // Current data
    core::mesh::Mesh currentMesh_;
    core::slicing::SlicingResult slicingResult_;  // ← YENİ! Slicing sonuçları

    // Helper
    void updateMeshInfo();
    void setUiBusy(bool busy);  // ← Arka plan işlemi sırasında UI'ı kilitle/aç
};

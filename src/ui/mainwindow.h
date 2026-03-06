#pragma once

#include <QMainWindow>
#include <memory>
#include "core/mesh/mesh.h"
#include "core/slicing/Slicer.h"
#include "io/loading/ILoadingStrategy.h"
#include "io/loading/AsyncLoadingStrategy.h"
#include "core/buildplate/BuildPlate.h"

// Forward declarations
namespace rendering {
class MeshRenderer;
}

class QPushButton;
class QLabel;
class QSlider;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadModel();
    void onLoadModelAsync();
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
    void onShowLayers();
    void onLayerChanged(int);

    // Cached loading - YENİ! ← SADECE BUNLARI EKLEDİK
    void onLoadModelCachedSync();
    void onLoadModelCachedAsync();

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
    QPushButton* btnShowLayers_;

    // Layer slider
    QSlider* sliderLayer_;
    QLabel* labelCurrentLayer_;

    // Status labels
    QLabel* labelTriangleCount_;
    QLabel* labelVertexCount_;
    QLabel* labelLayerCount_;

    // Current data
    core::mesh::Mesh currentMesh_;
    core::slicing::SlicingResult slicingResult_;

    // Loading strategies
    std::unique_ptr<io::loading::ILoadingStrategy> m_loadingStrategy;
    std::unique_ptr<io::loading::ILoadingStrategy> m_cachedSyncStrategy;
    std::unique_ptr<io::loading::ILoadingStrategy> m_cachedAsyncStrategy;
    std::unique_ptr<io::loading::AsyncLoadingStrategy> m_activeAsyncStrategy;
    std::shared_ptr<core::buildplate::BuildPlate> currentPlate_;

    // Helper
    void updateMeshInfo();
    void onPlateCreated(std::shared_ptr<core::buildplate::BuildPlate> plate);
};

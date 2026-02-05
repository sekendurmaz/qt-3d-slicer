#pragma once

#include <QMainWindow>
#include "core/mesh/mesh.h"

// Forward declarations
namespace rendering {
class MeshRenderer;
}

class QPushButton;

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

private:
    // UI components (no Designer!)
    rendering::MeshRenderer* meshRenderer_;
    QPushButton* btnLoad_;
    QPushButton* btnWireframe_;
    QPushButton* btnSolid_;
    QPushButton* btnReset_;

    // Current mesh
    core::mesh::Mesh currentMesh_;
};

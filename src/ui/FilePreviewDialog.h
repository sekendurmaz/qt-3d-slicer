#pragma once

#include <QDialog>
#include <QFileDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QString>
#include "core/mesh/mesh.h"

// Forward declaration
namespace rendering {
class MeshRenderer;
}

class FilePreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilePreviewDialog(QWidget *parent = nullptr);
    ~FilePreviewDialog();

    QString getSelectedFile() const;
    int exec() override;

private slots:
    void onFileSelectionChanged(const QString &path);

private:
    void setupUI();
    void updatePreview(const QString &filePath);
    void clearPreview();

    QFileDialog* fileDialog_;
    rendering::MeshRenderer* previewRenderer_;
    
    QLabel* labelFileName_;
    QLabel* labelFileSize_;
    QLabel* labelTriangles_;
    QLabel* labelVertices_;
    QLabel* labelDimensions_;
    
    QString selectedFile_;
    core::mesh::Mesh previewMesh_;
};

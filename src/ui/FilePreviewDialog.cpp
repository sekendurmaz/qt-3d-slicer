#include "FilePreviewDialog.h"
#include "rendering/MeshRenderer.h"
#include "io/models/common/ModelFactory.h"
#include "core/mesh/MeshAnalyzer.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QFileInfo>
#include <QDebug>
#include <QMessageBox>

FilePreviewDialog::FilePreviewDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

FilePreviewDialog::~FilePreviewDialog()
{
    // Qt handles cleanup automatically (parent-child relationship)
}

void FilePreviewDialog::setupUI()
{
    setWindowTitle("Select 3D Model with Preview");
    resize(1200, 700);

    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Splitter to divide file dialog and preview
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    // Left side: File dialog (embedded)
    fileDialog_ = new QFileDialog(this);
    fileDialog_->setFileMode(QFileDialog::ExistingFile);
    fileDialog_->setNameFilter("3D Models (*.stl *.obj *.3mf);;STL Files (*.stl);;OBJ Files (*.obj);;3MF Files (*.3mf);;All Files (*)");
    fileDialog_->setOption(QFileDialog::DontUseNativeDialog, true);
    fileDialog_->setViewMode(QFileDialog::Detail);
    
    // Remove default buttons from embedded dialog
    QList<QPushButton*> buttons = fileDialog_->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        button->hide();
    }

    splitter->addWidget(fileDialog_);

    // Right side: Preview panel
    QWidget* previewPanel = new QWidget(this);
    QVBoxLayout* previewLayout = new QVBoxLayout(previewPanel);
    
    // Preview renderer
    QGroupBox* rendererGroup = new QGroupBox("3D Preview", this);
    QVBoxLayout* rendererLayout = new QVBoxLayout(rendererGroup);
    
    previewRenderer_ = new rendering::MeshRenderer(this);
    previewRenderer_->setMinimumSize(400, 400);
    rendererLayout->addWidget(previewRenderer_);
    
    previewLayout->addWidget(rendererGroup, 1);

    // File info panel
    QGroupBox* infoGroup = new QGroupBox("File Information", this);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    
    labelFileName_ = new QLabel("No file selected", this);
    labelFileName_->setWordWrap(true);
    labelFileName_->setStyleSheet("QLabel { font-weight: bold; }");
    
    labelFileSize_ = new QLabel("Size: -", this);
    labelTriangles_ = new QLabel("Triangles: -", this);
    labelVertices_ = new QLabel("Vertices: -", this);
    labelDimensions_ = new QLabel("Dimensions: -", this);
    
    infoLayout->addWidget(labelFileName_);
    infoLayout->addWidget(labelFileSize_);
    infoLayout->addWidget(labelTriangles_);
    infoLayout->addWidget(labelVertices_);
    infoLayout->addWidget(labelDimensions_);
    infoLayout->addStretch();
    
    previewLayout->addWidget(infoGroup);

    splitter->addWidget(previewPanel);
    
    // Set splitter sizes (60% file dialog, 40% preview)
    splitter->setStretchFactor(0, 6);
    splitter->setStretchFactor(1, 4);

    mainLayout->addWidget(splitter);

    // Dialog buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* btnOpen = new QPushButton("Open", this);
    QPushButton* btnCancel = new QPushButton("Cancel", this);
    
    btnOpen->setMinimumHeight(35);
    btnCancel->setMinimumHeight(35);
    
    buttonLayout->addWidget(btnOpen);
    buttonLayout->addWidget(btnCancel);
    
    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(fileDialog_, &QFileDialog::currentChanged, 
            this, &FilePreviewDialog::onFileSelectionChanged);
    connect(btnOpen, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

QString FilePreviewDialog::getSelectedFile() const
{
    return selectedFile_;
}

int FilePreviewDialog::exec()
{
    clearPreview();
    return QDialog::exec();
}

void FilePreviewDialog::onFileSelectionChanged(const QString &path)
{
    qDebug() << "File selection changed:" << path;
    
    // Check if path is empty or is a directory
    if (path.isEmpty()) {
        clearPreview();
        return;
    }
    
    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || fileInfo.isDir()) {
        clearPreview();
        return;
    }
    
    // Check if file has valid extension
    QString ext = fileInfo.suffix().toLower();
    if (ext != "stl" && ext != "obj" && ext != "3mf") {
        clearPreview();
        labelFileName_->setText(fileInfo.fileName());
        labelFileSize_->setText(QString("Size: %1 KB").arg(fileInfo.size() / 1024.0, 0, 'f', 2));
        labelTriangles_->setText("Unsupported file format");
        return;
    }
    
    selectedFile_ = path;
    updatePreview(path);
}

void FilePreviewDialog::updatePreview(const QString &filePath)
{
    qDebug() << "Updating preview for:" << filePath;
    
    QFileInfo fileInfo(filePath);
    
    // Update file name and size
    labelFileName_->setText(fileInfo.fileName());
    labelFileSize_->setText(QString("Size: %1 KB").arg(fileInfo.size() / 1024.0, 0, 'f', 2));
    
    try {
        // Load mesh
        previewMesh_ = io::models::ModelFactory::loadModel(filePath.toStdString());
        
        // Render mesh
        previewRenderer_->setMesh(previewMesh_);
        
        // Analyze mesh
        core::mesh::MeshAnalyzer analyzer;
        auto stats = analyzer.analyze(previewMesh_);
        
        // Update labels
        labelTriangles_->setText(QString("Triangles: %1").arg(stats.triangleCount));
        labelVertices_->setText(QString("Vertices: ~%1").arg(stats.vertexCount));
        labelDimensions_->setText(QString("Dimensions: %1 x %2 x %3 mm")
                                     .arg(stats.dimensions.x, 0, 'f', 1)
                                     .arg(stats.dimensions.y, 0, 'f', 1)
                                     .arg(stats.dimensions.z, 0, 'f', 1));
        
        qDebug() << "Preview updated successfully";
        
    } catch (const std::exception& e) {
        qDebug() << "Error loading preview:" << e.what();
        clearPreview();
        labelTriangles_->setText("Error loading preview");
        labelVertices_->setText(QString("Error: %1").arg(e.what()));
        labelDimensions_->setText("-");
    }
}

void FilePreviewDialog::clearPreview()
{
    previewMesh_ = core::mesh::Mesh();
    previewRenderer_->setMesh(previewMesh_);
    
    labelFileName_->setText("No file selected");
    labelFileSize_->setText("Size: -");
    labelTriangles_->setText("Triangles: -");
    labelVertices_->setText("Vertices: -");
    labelDimensions_->setText("Dimensions: -");
}

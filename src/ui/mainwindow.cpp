#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

// IO katmanını dahil et
#include "io/models/common/ModelFactory.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::clicked,
            this, &MainWindow::onPushButtonClicked);
    // Pencere başlığı
    setWindowTitle("3D Model Reader");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onPushButtonClicked()
{
    // Dosya seçme dialogu - Tüm formatları destekle
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "3D Model Dosyası Seç",
        "",
        "3D Models (*.stl *.obj *.3mf);;STL Files (*.stl);;OBJ Files (*.obj);;3MF Files (*.3mf);;All Files (*)"
        );

    // Kullanıcı iptal ettiyse
    if (fileName.isEmpty()) {
        return;
    }

    qDebug() << "Seçilen dosya:" << fileName;

    // Model yükleme
    try {
        // ModelFactory ile dosyayı yükle
        auto mesh = io::models::ModelFactory::loadModel(
            fileName.toStdString()
            );

        // Başarı mesajı
        QString message = QString("Model başarıyla yüklendi!\n\n"
                                  "Dosya: %1\n"
                                  "Triangle sayısı: %2")
                              .arg(fileName)
                              .arg(mesh.triangleCount());

        QMessageBox::information(this, "Başarılı", message);

        qDebug() << "Mesh yüklendi:"
                 << mesh.triangleCount() << "triangles";

        // TODO: Mesh'i render et veya işle
        // displayMesh(mesh);

    } catch (const std::exception& e) {
        // Hata mesajı
        QString errorMsg = QString("Model yüklenirken hata oluştu:\n\n%1")
                               .arg(e.what());

        QMessageBox::critical(this, "Hata", errorMsg);

        qDebug() << "Hata:" << e.what();
    }
}



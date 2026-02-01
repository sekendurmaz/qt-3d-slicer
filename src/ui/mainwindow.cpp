#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "QDebug"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked,
            this, &MainWindow::onPushButtonClicked);
    // Pencere başlığı
    setWindowTitle("STL File Reader");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onPushButtonClicked()
{
    // Dosya seçme dialogu
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "STL Dosyası Seç",
        "",
        "STL Files (*.stl);;All Files (*)"
        );

    // Kullanıcı iptal ettiyse
    if (fileName.isEmpty()) {
        return;
    }

    qDebug() << "Seçilen dosya:" << fileName;

    // STL dosyasını oku
    if (m_stlReader.readSTL(fileName.toStdString(), m_currentMesh)) {
        // Başarılı
        QString info = QString("✓ STL dosyası yüklendi!\n\n")
                       + QString("Üçgen sayısı: %1\n").arg(m_currentMesh.triangleCount())
                       + QString("Dosya: %1").arg(fileName);

        ui->infoLabel->setText(info);

        QMessageBox::information(this, "Başarılı",
                                 QString("STL dosyası başarıyla yüklendi!\n\nÜçgen sayısı: %1")
                                     .arg(m_currentMesh.triangleCount()));

        // Debug: İlk üçgenin bilgileri
        if (m_currentMesh.triangleCount() > 0) {
            const Triangle& tri = m_currentMesh.triangles[0];

            qDebug() << "İlk üçgen:";
            qDebug() << "  Normal:" << tri.normal.x << tri.normal.y << tri.normal.z;
            qDebug() << "  Vertex1:" << tri.vertex1.x << tri.vertex1.y << tri.vertex1.z;
            qDebug() << "  Vertex2:" << tri.vertex2.x << tri.vertex2.y << tri.vertex2.z;
            qDebug() << "  Vertex3:" << tri.vertex3.x << tri.vertex3.y << tri.vertex3.z;
        }
    } else {
        // Hata
        QString error =
            "✗ STL dosyası okunamadı!\n\n" +
            QString::fromStdString(m_stlReader.getLastError());
        ui->infoLabel->setText(error);
        QMessageBox::critical(this, "Hata", error);
    }
}

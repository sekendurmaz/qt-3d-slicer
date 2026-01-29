#include "stlreader.h"
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QDebug>

STLReader::STLReader()
{
}
bool STLReader::readSTL(const QString& filePath, STLMesh& mesh)
{
    mesh.clear();
    m_lastError.clear();

    // Dosya var mı kontrol et
    QFile file(filePath);
    if (!file.exists()) {
        m_lastError = "Dosya bulunamadı: " + filePath;
        return false;
    }

    // Binary mi ASCII mi?
    if (isBinarySTL(filePath)) {
        qDebug() << "Binary STL dosyası okunuyor...";
        return readBinarySTL(filePath, mesh);
    } else {
        qDebug() << "ASCII STL dosyası okunuyor...";
        return readASCIISTL(filePath, mesh);
    }
}

bool STLReader::isBinarySTL(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // İlk 5 byte'ı oku
    QByteArray header = file.read(5);
    file.close();

    // ASCII STL "solid" ile başlar
    return !(header.startsWith("solid"));
}

bool STLReader::readBinarySTL(const QString& filePath, STLMesh& mesh)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = "Dosya açılamadı: " + filePath;
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // 80 byte header (okumuyor, atlıyoruz)
    char header[80];
    stream.readRawData(header, 80);

    // Triangle sayısı
    quint32 triangleCount;
    stream >> triangleCount;

    qDebug() << "Triangle sayısı:" << triangleCount;

    // Her triangle'ı oku
    for (quint32 i = 0; i < triangleCount; ++i) {
        Triangle tri;

        // Normal vector
        float nx, ny, nz;
        stream >> nx >> ny >> nz;
        tri.normal = QVector3D(nx, ny, nz);

        // Vertex 1
        float v1x, v1y, v1z;
        stream >> v1x >> v1y >> v1z;
        tri.vertex1 = QVector3D(v1x, v1y, v1z);

        // Vertex 2
        float v2x, v2y, v2z;
        stream >> v2x >> v2y >> v2z;
        tri.vertex2 = QVector3D(v2x, v2y, v2z);

        // Vertex 3
        float v3x, v3y, v3z;
        stream >> v3x >> v3y >> v3z;
        tri.vertex3 = QVector3D(v3x, v3y, v3z);

        // Attribute byte count (kullanmıyoruz)
        quint16 attrByteCount;
        stream >> attrByteCount;

        mesh.triangles.append(tri);
    }

    file.close();

    qDebug() << "STL dosyası başarıyla okundu:" << mesh.triangleCount() << "üçgen";
    return true;
}

bool STLReader::readASCIISTL(const QString& filePath, STLMesh& mesh)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = "Dosya açılamadı: " + filePath;
        return false;
    }

    QTextStream stream(&file);
    QString line;
    Triangle currentTriangle;
    int vertexIndex = 0;

    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);

        if (parts.isEmpty()) continue;

        if (parts[0] == "solid") {
            if (parts.size() > 1) {
                mesh.name = parts[1];
            }
        }
        else if (parts[0] == "facet" && parts.size() >= 5) {
            // Normal vector
            currentTriangle.normal = QVector3D(
                parts[2].toFloat(),
                parts[3].toFloat(),
                parts[4].toFloat()
                );
            vertexIndex = 0;
        }
        else if (parts[0] == "vertex" && parts.size() >= 4) {
            QVector3D vertex(
                parts[1].toFloat(),
                parts[2].toFloat(),
                parts[3].toFloat()
                );

            if (vertexIndex == 0) currentTriangle.vertex1 = vertex;
            else if (vertexIndex == 1) currentTriangle.vertex2 = vertex;
            else if (vertexIndex == 2) currentTriangle.vertex3 = vertex;

            vertexIndex++;
        }
        else if (parts[0] == "endfacet") {
            mesh.triangles.append(currentTriangle);
        }
    }

    file.close();

    qDebug() << "ASCII STL dosyası başarıyla okundu:" << mesh.triangleCount() << "üçgen";
    return true;
}

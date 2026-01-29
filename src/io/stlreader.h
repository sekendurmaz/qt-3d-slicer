#ifndef STLREADER_H
#define STLREADER_H

#include <QString>
#include <QVector>
#include <QVector3D>

// Bir üçgen yapısı
struct Triangle {
    QVector3D normal;
    QVector3D vertex1;
    QVector3D vertex2;
    QVector3D vertex3;
};

struct STLMesh {
    QString name;
    QVector<Triangle> triangles;

    void clear() {
        name.clear();
        triangles.clear();
    }

    int triangleCount() const {
        return triangles.size();
    }
};

class STLReader
{
public:
    STLReader();

    // STL dosyası okuma
    bool readSTL(const QString& filePath, STLMesh& mesh);

    // Hata mesajı
    QString getLastError() const { return m_lastError; }

private:
    // Binary STL okuma
    bool readBinarySTL(const QString& filePath, STLMesh& mesh);

    // ASCII STL okuma
    bool readASCIISTL(const QString& filePath, STLMesh& mesh);

    // Binary mi ASCII mi kontrol et
    bool isBinarySTL(const QString& filePath);

    QString m_lastError;
};

#endif // STLREADER_H

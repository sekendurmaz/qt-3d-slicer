#include "ObjReader.h"
#include "core/geometry/triangle.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace io {
namespace models {
namespace obj {

core::mesh::Mesh ObjReader::read(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file)
    {
        throw std::runtime_error("Cannot open OBJ file: " + filepath);
    }

    core::mesh::Mesh mesh;
    std::vector<core::geometry::Vec3> vertices;
    std::vector<core::geometry::Vec3> normals;

    std::string line;
    while (std::getline(file, line))
    {
        // Boş satır veya comment
        if (line.empty() || line[0] == '#')
            continue;

        // Satırı kelimelerine ayır
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v")
        {
            // Vertex
            vertices.push_back(parseVertex(line));
        }
        else if (prefix == "vn")
        {
            // Vertex normal
            normals.push_back(parseNormal(line));
        }
        else if (prefix == "f")
        {
            // Face
            parseFace(line, vertices, normals, mesh);
        }
        // vt (texture), mtllib, usemtl vs. şimdilik ignore et
    }

    return mesh;
}

bool ObjReader::canRead(const std::string& filepath) const
{
    if (filepath.length() < 4)
        return false;

    std::string ext = filepath.substr(filepath.length() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return ext == ".obj";
}

std::string ObjReader::getFormatName() const
{
    return "OBJ (Wavefront)";
}

core::geometry::Vec3 ObjReader::parseVertex(const std::string& line)
{
    // Format: v x y z [w]
    float x, y, z;
    sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
    return core::geometry::Vec3(x, y, z);
}

core::geometry::Vec3 ObjReader::parseNormal(const std::string& line)
{
    // Format: vn x y z
    float x, y, z;
    sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z);
    return core::geometry::Vec3(x, y, z);
}

void ObjReader::parseFace(const std::string& line,
                          const std::vector<core::geometry::Vec3>& vertices,
                          const std::vector<core::geometry::Vec3>& normals,
                          core::mesh::Mesh& mesh)
{
    // Face formatları:
    // f v1 v2 v3 ...
    // f v1/vt1 v2/vt2 v3/vt3 ...
    // f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ...
    // f v1//vn1 v2//vn2 v3//vn3 ...

    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;  // "f" kelimesini atla

    std::vector<int> faceVertices;
    std::string token;

    // Tüm vertex index'lerini oku
    while (iss >> token)
    {
        int vertexIndex = parseFaceIndex(token);
        faceVertices.push_back(vertexIndex);
    }

    // Face en az 3 vertex olmalı (triangle)
    if (faceVertices.size() < 3)
    {
        return;  // Geçersiz face, atla
    }

    // Triangulation: Fan triangulation kullan
    // Örnek: Face 4 vertex → Triangle 1: (0,1,2), Triangle 2: (0,2,3)
    for (size_t i = 1; i + 1 < faceVertices.size(); ++i)
    {
        int idx0 = faceVertices[0];
        int idx1 = faceVertices[i];
        int idx2 = faceVertices[i + 1];

        // Index'ler 1-based, vertices vector'ü 0-based
        // Range check
        if (idx0 >= 0 && idx0 < static_cast<int>(vertices.size()) &&
            idx1 >= 0 && idx1 < static_cast<int>(vertices.size()) &&
            idx2 >= 0 && idx2 < static_cast<int>(vertices.size()))
        {
            const auto& v0 = vertices[idx0];
            const auto& v1 = vertices[idx1];
            const auto& v2 = vertices[idx2];

            // Normal hesapla (cross product)
            // (v1-v0) x (v2-v0)
            float dx1 = v1.x - v0.x;
            float dy1 = v1.y - v0.y;
            float dz1 = v1.z - v0.z;

            float dx2 = v2.x - v0.x;
            float dy2 = v2.y - v0.y;
            float dz2 = v2.z - v0.z;

            float nx = dy1 * dz2 - dz1 * dy2;
            float ny = dz1 * dx2 - dx1 * dz2;
            float nz = dx1 * dy2 - dy1 * dx2;

            core::geometry::Vec3 normal(nx, ny, nz);

            // Triangle ekle
            mesh.addTriangle(v0, v1, v2, normal);
        }
    }
}

int ObjReader::parseFaceIndex(const std::string& token)
{
    // Format: v, v/vt, v/vt/vn, v//vn
    // Biz sadece ilk index'i (vertex) kullanacağız

    // İlk '/' karakterine kadar olan kısmı al
    size_t pos = token.find('/');
    std::string vertexStr;

    if (pos != std::string::npos)
    {
        vertexStr = token.substr(0, pos);
    }
    else
    {
        vertexStr = token;
    }

    // String'i int'e çevir
    int index = std::stoi(vertexStr);

    // OBJ index'leri 1'den başlar, biz 0'dan başlayan index istiyoruz
    return index - 1;
}

} // namespace obj
} // namespace models
} // namespace io

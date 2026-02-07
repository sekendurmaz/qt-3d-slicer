#include "StlReader.h"
#include "core/geometry/vec3.h"

#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <cstdint>

namespace io {
namespace models {
namespace stl {

core::mesh::Mesh StlReader::read(const std::string& filepath)
{
    // Dosyanın var olup olmadığını kontrol et
    std::ifstream file(filepath, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    file.close();

    // Binary mi ASCII mi tespit et
    if (isBinaryFormat(filepath))
    {
        return readBinary(filepath);
    }
    else
    {
        return readAscii(filepath);
    }
}

bool StlReader::canRead(const std::string& filepath) const
{
    // Uzantıyı kontrol et
    if (filepath.length() < 4)
        return false;

    std::string ext = filepath.substr(filepath.length() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return ext == ".stl";
}

std::string StlReader::getFormatName() const
{
    return "STL (Stereolithography)";
}

core::mesh::Mesh StlReader::readBinary(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Cannot open binary STL file: " + filepath);
    }

    core::mesh::Mesh mesh;

    // 80 byte header (kullanmıyoruz, atla)
    char header[80];
    file.read(header, 80);

    // Triangle sayısını oku (4 byte, little-endian uint32)
    uint32_t triangleCount = 0;
    file.read(reinterpret_cast<char*>(&triangleCount), sizeof(uint32_t));

    // ⭐ PERFORMANS: Tam triangle sayısını biliyoruz, reserve et!
    mesh.reserve(triangleCount);

    // Triangle'ları oku
    for (uint32_t i = 0; i < triangleCount; ++i)
    {
        // Normal vector (12 byte: 3 float)
        float nx, ny, nz;
        file.read(reinterpret_cast<char*>(&nx), sizeof(float));
        file.read(reinterpret_cast<char*>(&ny), sizeof(float));
        file.read(reinterpret_cast<char*>(&nz), sizeof(float));

        // Vertex 1 (12 byte: 3 float)
        float v1x, v1y, v1z;
        file.read(reinterpret_cast<char*>(&v1x), sizeof(float));
        file.read(reinterpret_cast<char*>(&v1y), sizeof(float));
        file.read(reinterpret_cast<char*>(&v1z), sizeof(float));

        // Vertex 2 (12 byte: 3 float)
        float v2x, v2y, v2z;
        file.read(reinterpret_cast<char*>(&v2x), sizeof(float));
        file.read(reinterpret_cast<char*>(&v2y), sizeof(float));
        file.read(reinterpret_cast<char*>(&v2z), sizeof(float));

        // Vertex 3 (12 byte: 3 float)
        float v3x, v3y, v3z;
        file.read(reinterpret_cast<char*>(&v3x), sizeof(float));
        file.read(reinterpret_cast<char*>(&v3y), sizeof(float));
        file.read(reinterpret_cast<char*>(&v3z), sizeof(float));

        // Attribute byte count (2 byte, kullanmıyoruz)
        uint16_t attributeByteCount = 0;
        file.read(reinterpret_cast<char*>(&attributeByteCount), sizeof(uint16_t));

        // Triangle oluştur ve mesh'e ekle
        core::geometry::Vec3 normal(nx, ny, nz);
        core::geometry::Vec3 v1(v1x, v1y, v1z);
        core::geometry::Vec3 v2(v2x, v2y, v2z);
        core::geometry::Vec3 v3(v3x, v3y, v3z);

        mesh.addTriangle(v1, v2, v3, normal);
    }

    if (!file)
    {
        throw std::runtime_error("Error reading binary STL file: " + filepath);
    }

    return mesh;
}

core::mesh::Mesh StlReader::readAscii(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file)
    {
        throw std::runtime_error("Cannot open ASCII STL file: " + filepath);
    }

    core::mesh::Mesh mesh;
    std::string line;

    // "solid" satırını atla
    std::getline(file, line);

    while (std::getline(file, line))
    {
        // "endsolid" satırını gördüysek bitir
        if (line.find("endsolid") != std::string::npos)
        {
            break;
        }

        // "facet normal" satırını ara
        if (line.find("facet normal") != std::string::npos)
        {
            // Normal vector'ü parse et
            float nx, ny, nz;
            sscanf(line.c_str(), " facet normal %f %f %f", &nx, &ny, &nz);
            core::geometry::Vec3 normal(nx, ny, nz);

            // "outer loop" satırını atla
            std::getline(file, line);

            // Vertex 1
            std::getline(file, line);
            float v1x, v1y, v1z;
            sscanf(line.c_str(), " vertex %f %f %f", &v1x, &v1y, &v1z);
            core::geometry::Vec3 v1(v1x, v1y, v1z);

            // Vertex 2
            std::getline(file, line);
            float v2x, v2y, v2z;
            sscanf(line.c_str(), " vertex %f %f %f", &v2x, &v2y, &v2z);
            core::geometry::Vec3 v2(v2x, v2y, v2z);

            // Vertex 3
            std::getline(file, line);
            float v3x, v3y, v3z;
            sscanf(line.c_str(), " vertex %f %f %f", &v3x, &v3y, &v3z);
            core::geometry::Vec3 v3(v3x, v3y, v3z);

            // Triangle ekle
            mesh.addTriangle(v1, v2, v3, normal);

            // "endloop" satırını atla
            std::getline(file, line);

            // "endfacet" satırını atla
            std::getline(file, line);
        }
    }

    return mesh;
}

bool StlReader::isBinaryFormat(const std::string& filepath) const
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file)
        return false;

    // İlk 80 byte header'ı oku
    char header[80];
    file.read(header, 80);

    // ASCII STL "solid" ile başlar
    // Ama binary STL'in header'ında da "solid" olabilir!
    // Bu yüzden daha güvenilir kontrol: 84. byte'tan triangle count oku
    
    // Triangle count'u oku
    uint32_t triangleCount = 0;
    file.read(reinterpret_cast<char*>(&triangleCount), sizeof(uint32_t));

    // Dosya boyutunu kontrol et
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();

    // Binary STL boyutu = 80 (header) + 4 (count) + 50 * triangleCount
    size_t expectedSize = 80 + 4 + (50 * triangleCount);

    // Boyut uyuşuyorsa binary
    return (fileSize == expectedSize);
}

} // namespace stl
} // namespace models
} // namespace io

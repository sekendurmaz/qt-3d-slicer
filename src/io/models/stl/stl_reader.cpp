#include "stl_reader.h"

#include <fstream>      // std::ifstream
#include <sstream>      // std::istringstream
#include <string>
#include <cstdint>      // uint32_t, uint16_t
#include <cstring>      // std::memcmp
#include <filesystem>   // std::filesystem::exists (C++17)

namespace
{
// STL binary: 80-byte header + 4-byte triangle count + N * 50 bytes
// 50 bytes per triangle: normal(12) + v1(12) + v2(12) + v3(12) + attr(2)

bool readFloatLE(std::istream& in, float& out)
{
    // STL is little-endian floats
    static_assert(sizeof(float) == 4, "float must be 4 bytes");
    char buf[4];
    if (!in.read(buf, 4)) return false;

    // Assuming platform is little-endian (Windows x86/x64 usually). For strict portability, swap if needed.
    std::memcpy(&out, buf, 4);
    return true;
}

bool readU32LE(std::istream& in, std::uint32_t& out)
{
    char buf[4];
    if (!in.read(buf, 4)) return false;
    out = static_cast<std::uint32_t>(
        (static_cast<unsigned char>(buf[0])      ) |
        (static_cast<unsigned char>(buf[1]) <<  8) |
        (static_cast<unsigned char>(buf[2]) << 16) |
        (static_cast<unsigned char>(buf[3]) << 24)
        );
    return true;
}

bool readU16LE(std::istream& in, std::uint16_t& out)
{
    char buf[2];
    if (!in.read(buf, 2)) return false;
    out = static_cast<std::uint16_t>(
        (static_cast<unsigned char>(buf[0])     ) |
        (static_cast<unsigned char>(buf[1]) << 8)
        );
    return true;
}

std::string trim(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}
}

bool STLReader::readSTL(const std::string& filePath, Mesh& mesh)
{
    mesh.clear();            // sende varsa
    m_lastError.clear();

    // Dosya var mı?
    std::error_code ec;
    if (!std::filesystem::exists(filePath, ec) || ec)
    {
        m_lastError = "Dosya bulunamadi: " + filePath;
        return false;
    }

    // Binary mi ASCII mi?
    if (isBinarySTL(filePath))
        return readBinarySTL(filePath, mesh);

    return readASCIISTL(filePath, mesh);
}

bool STLReader::isBinarySTL(const std::string& filePath)
{
    std::ifstream in(filePath, std::ios::binary);
    if (!in) return false;

    // Basit kontrol: ilk 5 byte "solid" ise ASCII olabilir.
    // Not: Binary STL header'ı da "solid" ile başlayabilir (nadir ama olur).
    // Bu yüzden binary dosya uzunluğu kontrolünü de ekliyoruz.
    char first5[5]{};
    if (!in.read(first5, 5)) return false;

    const bool startsWithSolid = (std::memcmp(first5, "solid", 5) == 0);

    // Dosya boyutundan da kontrol edelim (daha güvenilir):
    // 80 + 4 + n*50 = file_size olmalı (attr dahil)
    std::error_code ec;
    const auto size = std::filesystem::file_size(filePath, ec);
    if (ec) return !startsWithSolid; // boyutu alamazsak eski mantığa dön

    // Triangle count'u okumak için 80+4 offset'e gidelim:
    in.seekg(80, std::ios::beg);
    std::uint32_t triCount = 0;
    if (!readU32LE(in, triCount))
        return !startsWithSolid;

    const std::uint64_t expected = 84ull + static_cast<std::uint64_t>(triCount) * 50ull;
    const bool sizeMatchesBinaryLayout = (expected == size);

    if (sizeMatchesBinaryLayout) return true;
    // Boyut uyuşmuyorsa ASCII ihtimali yüksek; "solid" ile başlıyorsa ASCII diyelim.
    return !startsWithSolid;
}

bool STLReader::readBinarySTL(const std::string& filePath, Mesh& mesh)
{
    std::ifstream in(filePath, std::ios::binary);
    if (!in)
    {
        m_lastError = "Dosya acilamadi: " + filePath;
        return false;
    }

    // 80 byte header
    char header[80];
    if (!in.read(header, 80))
    {
        m_lastError = "Binary STL header okunamadi: " + filePath;
        return false;
    }

    // Triangle sayısı
    std::uint32_t triangleCount = 0;
    if (!readU32LE(in, triangleCount))
    {
        m_lastError = "Triangle sayisi okunamadi: " + filePath;
        return false;
    }

    // Performans: üçgen sayısı kadar yer ayır
    mesh.triangles.clear();
    mesh.triangles.reserve(static_cast<size_t>(triangleCount));

    for (std::uint32_t i = 0; i < triangleCount; ++i)
    {
        Triangle tri;

        float nx, ny, nz;
        float v1x, v1y, v1z;
        float v2x, v2y, v2z;
        float v3x, v3y, v3z;

        if (!readFloatLE(in, nx) || !readFloatLE(in, ny) || !readFloatLE(in, nz) ||
            !readFloatLE(in, v1x) || !readFloatLE(in, v1y) || !readFloatLE(in, v1z) ||
            !readFloatLE(in, v2x) || !readFloatLE(in, v2y) || !readFloatLE(in, v2z) ||
            !readFloatLE(in, v3x) || !readFloatLE(in, v3y) || !readFloatLE(in, v3z))
        {
            m_lastError = "Binary STL veri eksik/bozuk (triangle " + std::to_string(i) + "): " + filePath;
            return false;
        }

        tri.normal  = Vec3(nx, ny, nz);
        tri.vertex1 = Vec3(v1x, v1y, v1z);
        tri.vertex2 = Vec3(v2x, v2y, v2z);
        tri.vertex3 = Vec3(v3x, v3y, v3z);

        std::uint16_t attr = 0;
        if (!readU16LE(in, attr))
        {
            m_lastError = "Binary STL attribute byte count okunamadi (triangle " + std::to_string(i) + "): " + filePath;
            return false;
        }

        mesh.triangles.push_back(tri);
    }

    // (İstersen) bounds hesapla:
    // mesh.computeBounds();

    return true;
}

bool STLReader::readASCIISTL(const std::string& filePath, Mesh& mesh)
{
    std::ifstream in(filePath);
    if (!in)
    {
        m_lastError = "Dosya acilamadi: " + filePath;
        return false;
    }

    std::string line;
    Triangle currentTriangle{};
    int vertexIndex = 0;

    while (std::getline(in, line))
    {
        line = trim(line);
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string token;
        ss >> token;
        if (token.empty()) continue;

        if (token == "solid")
        {
            // solid <name>
            std::string name;
            std::getline(ss, name);
            name = trim(name);
            if (!name.empty())
            {
                mesh.name = name; // Mesh.name std::string ise
            }
        }
        else if (token == "facet")
        {
            // facet normal nx ny nz
            std::string sub;
            ss >> sub;
            if (sub == "normal")
            {
                float nx, ny, nz;
                if (!(ss >> nx >> ny >> nz))
                {
                    m_lastError = "ASCII STL facet normal parse hatasi: " + filePath;
                    return false;
                }
                currentTriangle.normal = Vec3(nx, ny, nz);
                vertexIndex = 0;
            }
        }
        else if (token == "vertex")
        {
            float x, y, z;
            if (!(ss >> x >> y >> z))
            {
                m_lastError = "ASCII STL vertex parse hatasi: " + filePath;
                return false;
            }

            Vec3 v(x, y, z);
            if (vertexIndex == 0) currentTriangle.vertex1 = v;
            else if (vertexIndex == 1) currentTriangle.vertex2 = v;
            else if (vertexIndex == 2) currentTriangle.vertex3 = v;

            ++vertexIndex;
        }
        else if (token == "endfacet")
        {
            // 3 vertex gelmemişse de ekleme; dosya bozuk olabilir.
            if (vertexIndex >= 3)
            {
                mesh.triangles.push_back(currentTriangle);
            }
            else
            {
                // İstersen burada hata yapabilirsin; ben soft davranıyorum
                // m_lastError = "ASCII STL endfacet ama 3 vertex yok: " + filePath;
                // return false;
            }
        }
    }

    return true;
}

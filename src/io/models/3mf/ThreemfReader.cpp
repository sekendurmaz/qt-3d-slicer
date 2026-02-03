#include "ThreemfReader.h"
#include "core/geometry/vec3.h"

#include <QFile>
#include <QByteArray>
#include <QDataStream>
#include <QDebug>          // ← DEBUG için eklendi
#include <stdexcept>
#include <algorithm>
#include <zlib.h>

// TinyXML2
#include "tinyxml2.h"

namespace io {
namespace models {
namespace threemf {

core::mesh::Mesh ThreemfReader::read(const std::string& filepath)
{
    // 1. ZIP'ten XML çıkar
    std::string xmlContent = extractXmlFromZip(filepath);

    // 2. XML'i parse et ve mesh oluştur
    return parseXml(xmlContent);
}

bool ThreemfReader::canRead(const std::string& filepath) const
{
    if (filepath.length() < 4)
        return false;

    std::string ext = filepath.substr(filepath.length() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return ext == ".3mf";
}

std::string ThreemfReader::getFormatName() const
{
    return "3MF (3D Manufacturing Format)";
}

std::string ThreemfReader::extractXmlFromZip(const std::string& filepath)
{
    QFile file(QString::fromStdString(filepath));
    if (!file.open(QIODevice::ReadOnly))
    {
        throw std::runtime_error("Cannot open 3MF file: " + filepath);
    }

    QByteArray fileData = file.readAll();
    file.close();

    // ZIP signature kontrolü
    if (fileData.size() < 4 ||
        fileData[0] != 'P' || fileData[1] != 'K')
    {
        throw std::runtime_error("Invalid 3MF file (not a ZIP): " + filepath);
    }

    // End of Central Directory'yi bul (en sonda)
    // EOCD signature: 0x06054b50 (PK\x05\x06)
    int eocdOffset = -1;
    for (int i = fileData.size() - 22; i >= 0; i--)
    {
        if (fileData[i] == 0x50 &&
            fileData[i+1] == 0x4b &&
            fileData[i+2] == 0x05 &&  // ← 0x05 (EOCD), 0x01 değil!
            fileData[i+3] == 0x06)
        {
            eocdOffset = i;
            break;
        }
    }

    if (eocdOffset < 0)
    {
        throw std::runtime_error("Invalid 3MF: End of Central Directory not found");
    }

    // Central Directory offset'ini EOCD'den oku
    uint32_t cdOffset = static_cast<uint8_t>(fileData[eocdOffset+16]) |
                        (static_cast<uint8_t>(fileData[eocdOffset+17]) << 8) |
                        (static_cast<uint8_t>(fileData[eocdOffset+18]) << 16) |
                        (static_cast<uint8_t>(fileData[eocdOffset+19]) << 24);

    qDebug() << "=== 3MF Archive Contents ===";
    qDebug() << "EOCD at:" << eocdOffset << ", Central Directory at:" << cdOffset;

    // Central directory'den dosyaları tara
    int offset = cdOffset;
    int fileCount = 0;

    while (offset < fileData.size() - 46)
    {
        // Central directory file header signature: 0x02014b50
        if (fileData[offset] != 0x50 ||
            fileData[offset+1] != 0x4b ||
            fileData[offset+2] != 0x01 ||
            fileData[offset+3] != 0x02)
        {
            qDebug() << "No more central directory entries at offset" << offset;
            break;  // Central directory bitti
        }

        fileCount++;

        // Compressed size (offset +20, 4 bytes)
        uint32_t compSize = static_cast<uint8_t>(fileData[offset+20]) |
                            (static_cast<uint8_t>(fileData[offset+21]) << 8) |
                            (static_cast<uint8_t>(fileData[offset+22]) << 16) |
                            (static_cast<uint8_t>(fileData[offset+23]) << 24);

        // Uncompressed size (offset +24, 4 bytes)
        uint32_t uncompSize = static_cast<uint8_t>(fileData[offset+24]) |
                              (static_cast<uint8_t>(fileData[offset+25]) << 8) |
                              (static_cast<uint8_t>(fileData[offset+26]) << 16) |
                              (static_cast<uint8_t>(fileData[offset+27]) << 24);

        // Filename length (offset +28, 2 bytes)
        uint16_t filenameLen = static_cast<uint8_t>(fileData[offset+28]) |
                               (static_cast<uint8_t>(fileData[offset+29]) << 8);

        // Extra field length (offset +30, 2 bytes)
        uint16_t extraLen = static_cast<uint8_t>(fileData[offset+30]) |
                            (static_cast<uint8_t>(fileData[offset+31]) << 8);

        // Comment length (offset +32, 2 bytes)
        uint16_t commentLen = static_cast<uint8_t>(fileData[offset+32]) |
                              (static_cast<uint8_t>(fileData[offset+33]) << 8);

        // Compression method (offset +10, 2 bytes)
        uint16_t compressionMethod = static_cast<uint8_t>(fileData[offset+10]) |
                                     (static_cast<uint8_t>(fileData[offset+11]) << 8);

        // Local header offset (offset +42, 4 bytes)
        uint32_t localHeaderOffset = static_cast<uint8_t>(fileData[offset+42]) |
                                     (static_cast<uint8_t>(fileData[offset+43]) << 8) |
                                     (static_cast<uint8_t>(fileData[offset+44]) << 16) |
                                     (static_cast<uint8_t>(fileData[offset+45]) << 24);

        // Filename
        QString filename = QString::fromLatin1(
            fileData.mid(offset + 46, filenameLen)
            );

        qDebug() << fileCount << ":" << filename
                 << "| CompSize:" << compSize
                 << "| UncompSize:" << uncompSize
                 << "| Method:" << compressionMethod;

        // 3dmodel.model veya .model uzantılı dosyayı bul
        if (filename.endsWith(".model", Qt::CaseInsensitive))
        {
            qDebug() << "Found .model file! Extracting...";

            // Local header'a git
            int localOffset = localHeaderOffset;

            // Local header'dan filename ve extra field length'i al
            uint16_t localFilenameLen = static_cast<uint8_t>(fileData[localOffset+26]) |
                                        (static_cast<uint8_t>(fileData[localOffset+27]) << 8);

            uint16_t localExtraLen = static_cast<uint8_t>(fileData[localOffset+28]) |
                                     (static_cast<uint8_t>(fileData[localOffset+29]) << 8);

            // Data başlangıcı
            int dataStart = localOffset + 30 + localFilenameLen + localExtraLen;

            if (compressionMethod == 0)  // No compression
            {
                qDebug() << "Uncompressed, returning directly";
                return std::string(fileData.mid(dataStart, compSize).data(), compSize);
            }
            else if (compressionMethod == 8)  // DEFLATE
            {
                qDebug() << "DEFLATE compression, decompressing...";
                QByteArray compressed = fileData.mid(dataStart, compSize);
                QByteArray uncompressed;
                uncompressed.resize(uncompSize);

                // Raw DEFLATE decompression (zlib API)
                z_stream stream;
                stream.zalloc = Z_NULL;
                stream.zfree = Z_NULL;
                stream.opaque = Z_NULL;
                stream.avail_in = compressed.size();
                stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed.data()));
                stream.avail_out = uncompSize;
                stream.next_out = reinterpret_cast<Bytef*>(uncompressed.data());

                // Raw DEFLATE: -MAX_WBITS (no zlib wrapper)
                int ret = inflateInit2(&stream, -MAX_WBITS);
                if (ret != Z_OK)
                {
                    throw std::runtime_error("inflateInit2 failed");
                }

                ret = inflate(&stream, Z_FINISH);
                inflateEnd(&stream);

                if (ret != Z_STREAM_END)
                {
                    throw std::runtime_error(
                        "DEFLATE decompression failed: " + std::to_string(ret)
                        );
                }

                uncompressed.resize(stream.total_out);
                qDebug() << "Successfully decompressed!" << uncompressed.size() << "bytes";
                return std::string(uncompressed.data(), uncompressed.size());
            }
            else
            {
                throw std::runtime_error("Unsupported compression: " + std::to_string(compressionMethod));
            }
        }

        // Bir sonraki central directory entry
        offset += 46 + filenameLen + extraLen + commentLen;
        qDebug() << "Next offset:" << offset << ", fileData.size:" << fileData.size();
    }

    qDebug() << "=== End of archive (" << fileCount << "files), .model file NOT FOUND ===";
    throw std::runtime_error(".model file not found in 3MF");
}

core::mesh::Mesh ThreemfReader::parseXml(const std::string& xmlContent)
{
    core::mesh::Mesh mesh;

    tinyxml2::XMLDocument doc;
    if (doc.Parse(xmlContent.c_str()) != tinyxml2::XML_SUCCESS)
    {
        throw std::runtime_error("Failed to parse 3MF XML");
    }

    // <model> root element
    auto* modelElem = doc.FirstChildElement("model");
    if (!modelElem)
    {
        throw std::runtime_error("No <model> element in 3MF");
    }

    // <resources>
    auto* resourcesElem = modelElem->FirstChildElement("resources");
    if (!resourcesElem)
    {
        throw std::runtime_error("No <resources> in 3MF");
    }

    // <object type="model">
    for (auto* objElem = resourcesElem->FirstChildElement("object");
         objElem != nullptr;
         objElem = objElem->NextSiblingElement("object"))
    {
        const char* typeAttr = objElem->Attribute("type");
        if (!typeAttr || std::string(typeAttr) != "model")
            continue;

        // <mesh>
        auto* meshElem = objElem->FirstChildElement("mesh");
        if (!meshElem)
            continue;

        // <vertices>
        std::vector<core::geometry::Vec3> vertices;
        auto* verticesElem = meshElem->FirstChildElement("vertices");
        if (verticesElem)
        {
            for (auto* vElem = verticesElem->FirstChildElement("vertex");
                 vElem != nullptr;
                 vElem = vElem->NextSiblingElement("vertex"))
            {
                float x = vElem->FloatAttribute("x");
                float y = vElem->FloatAttribute("y");
                float z = vElem->FloatAttribute("z");
                vertices.emplace_back(x, y, z);
            }
        }

        // <triangles>
        auto* trianglesElem = meshElem->FirstChildElement("triangles");
        if (trianglesElem)
        {
            for (auto* triElem = trianglesElem->FirstChildElement("triangle");
                 triElem != nullptr;
                 triElem = triElem->NextSiblingElement("triangle"))
            {
                int v1 = triElem->IntAttribute("v1");
                int v2 = triElem->IntAttribute("v2");
                int v3 = triElem->IntAttribute("v3");

                // Range check
                if (v1 >= 0 && v1 < static_cast<int>(vertices.size()) &&
                    v2 >= 0 && v2 < static_cast<int>(vertices.size()) &&
                    v3 >= 0 && v3 < static_cast<int>(vertices.size()))
                {
                    // Normal hesapla (cross product)
                    const auto& p1 = vertices[v1];
                    const auto& p2 = vertices[v2];
                    const auto& p3 = vertices[v3];

                    float dx1 = p2.x - p1.x;
                    float dy1 = p2.y - p1.y;
                    float dz1 = p2.z - p1.z;

                    float dx2 = p3.x - p1.x;
                    float dy2 = p3.y - p1.y;
                    float dz2 = p3.z - p1.z;

                    float nx = dy1 * dz2 - dz1 * dy2;
                    float ny = dz1 * dx2 - dx1 * dz2;
                    float nz = dx1 * dy2 - dy1 * dx2;

                    core::geometry::Vec3 normal(nx, ny, nz);

                    mesh.addTriangle(p1, p2, p3, normal);
                }
            }
        }
    }

    return mesh;
}

} // namespace threemf
} // namespace models
} // namespace io

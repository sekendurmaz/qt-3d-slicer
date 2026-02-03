#pragma once

#include "io/models/common/IModelReader.h"
#include "core/mesh/mesh.h"
#include "core/geometry/vec3.h"
#include <string>
#include <vector>

namespace io {
namespace models {
namespace obj {

/**
 * @brief OBJ (Wavefront) format okuyucu
 *
 * Wavefront OBJ dosyalarını okur ve mesh oluşturur.
 * Sadece vertex ve face bilgilerini kullanır.
 * Texture ve material bilgilerini şimdilik yok sayar.
 */
class ObjReader : public IModelReader
{
public:
    ObjReader() = default;
    ~ObjReader() override = default;

    /**
     * @brief OBJ dosyasını okur
     * @param filepath OBJ dosya yolu
     * @return core::mesh::Mesh nesnesi
     * @throws std::runtime_error Dosya okunamazsa
     */
    core::mesh::Mesh read(const std::string& filepath) override;

    /**
     * @brief Dosyanın .obj uzantılı olup olmadığını kontrol eder
     */
    bool canRead(const std::string& filepath) const override;

    /**
     * @brief Format adını döndürür
     */
    std::string getFormatName() const override;

private:
    /**
     * @brief Vertex satırını parse eder (v x y z)
     */
    core::geometry::Vec3 parseVertex(const std::string& line);

    /**
     * @brief Normal satırını parse eder (vn x y z)
     */
    core::geometry::Vec3 parseNormal(const std::string& line);

    /**
     * @brief Face satırını parse eder ve triangle'lara çevirir
     * Face dörtgen veya n-gen olabilir, triangulation yapar
     */
    void parseFace(const std::string& line,
                   const std::vector<core::geometry::Vec3>& vertices,
                   const std::vector<core::geometry::Vec3>& normals,
                   core::mesh::Mesh& mesh);

    /**
     * @brief Face index'ini parse eder (v, v/vt, v/vt/vn, v//vn formatları)
     * @return Vertex index (0-based)
     */
    int parseFaceIndex(const std::string& token);
};

} // namespace obj
} // namespace models
} // namespace io

#pragma once

#include "io/models/common/IModelReader.h"
#include "core/mesh/mesh.h"
#include <string>

namespace io {
namespace models {
namespace stl {

/**
 * @brief STL (Stereolithography) format okuyucu
 * 
 * Hem binary hem de ASCII STL formatını destekler.
 * Format otomatik tespit edilir.
 */
class StlReader : public IModelReader
{
public:
    StlReader() = default;
    ~StlReader() override = default;

    /**
     * @brief STL dosyasını okur
     * @param filepath STL dosya yolu
     * @return core::mesh::Mesh nesnesi
     * @throws std::runtime_error Dosya okunamazsa
     */
    core::mesh::Mesh read(const std::string& filepath) override;

    /**
     * @brief Dosyanın .stl uzantılı olup olmadığını kontrol eder
     */
    bool canRead(const std::string& filepath) const override;

    /**
     * @brief Format adını döndürür
     */
    std::string getFormatName() const override;

private:
    /**
     * @brief Binary STL okur
     */
    core::mesh::Mesh readBinary(const std::string& filepath);

    /**
     * @brief ASCII STL okur
     */
    core::mesh::Mesh readAscii(const std::string& filepath);

    /**
     * @brief Dosyanın binary mi ASCII mi olduğunu tespit eder
     */
    bool isBinaryFormat(const std::string& filepath) const;
};

} // namespace stl
} // namespace models
} // namespace io
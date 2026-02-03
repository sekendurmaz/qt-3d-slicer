#pragma once

#include "core/mesh/mesh.h"
#include <string>

namespace io {
namespace models {

/**
 * @brief Tüm model reader'lar için ortak interface
 * 
 * Her format (STL, OBJ, 3MF) bu interface'i implement eder.
 * Polymorphism sayesinde ModelFactory farklı reader'ları kullanabilir.
 */
class IModelReader
{
public:
    virtual ~IModelReader() = default;

    /**
     * @brief Model dosyasını okur ve Mesh döndürür
     * @param filepath Okunacak dosyanın tam yolu
     * @return core::mesh::Mesh nesnesi
     * @throws std::runtime_error Dosya okunamazsa veya format hatalıysa
     */
    virtual core::mesh::Mesh read(const std::string& filepath) = 0;

    /**
     * @brief Bu reader'ın verilen dosyayı okuyup okuyamayacağını kontrol eder
     * @param filepath Kontrol edilecek dosya yolu
     * @return true Bu reader dosyayı okuyabilir
     * @return false Bu reader dosyayı okuyamaz
     */
    virtual bool canRead(const std::string& filepath) const = 0;

    /**
     * @brief Format adını döndürür (örn: "STL", "OBJ", "3MF")
     * @return Format adı
     */
    virtual std::string getFormatName() const = 0;
};

} // namespace models
} // namespace io
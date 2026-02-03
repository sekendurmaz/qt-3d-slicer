#pragma once

#include "IModelReader.h"
#include "core/mesh/mesh.h"
#include <memory>
#include <string>

namespace io {
namespace models {

/**
 * @brief Model dosyalarını yüklemek için Factory sınıfı
 * 
 * Dosya uzantısına göre uygun reader'ı seçer ve mesh yükler.
 * Kullanıcı için basit API sağlar.
 */
class ModelFactory
{
public:
    /**
     * @brief Dosya uzantısına göre uygun reader oluşturur
     * @param filepath Dosya yolu (uzantı tespit için)
     * @return Reader pointer (ownership caller'a geçer)
     * @throws std::runtime_error Desteklenmeyen format için
     */
    static std::unique_ptr<IModelReader> createReader(const std::string& filepath);

    /**
     * @brief Model dosyasını yükler (Facade pattern)
     * 
     * Tek satırda dosyayı yüklemek için kolaylık fonksiyonu:
     * auto mesh = ModelFactory::loadModel("model.stl");
     * 
     * @param filepath Yüklenecek dosya yolu
     * @return core::mesh::Mesh nesnesi
     * @throws std::runtime_error Dosya yüklenemezse
     */
    static core::mesh::Mesh loadModel(const std::string& filepath);

private:
    /**
     * @brief Dosya uzantısını döndürür (küçük harfe çevrilmiş)
     * @param filepath Dosya yolu
     * @return Uzantı (örn: ".stl", ".obj", ".3mf")
     */
    static std::string getExtension(const std::string& filepath);
};

} // namespace models
} // namespace io
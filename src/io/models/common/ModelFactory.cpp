#include "ModelFactory.h"
#include "io/models/stl/StlReader.h"
#include "io/models/obj/ObjReader.h"
#include "io/models/3mf/ThreemfReader.h"

#include <algorithm>
#include <stdexcept>

namespace io {
namespace models {

std::unique_ptr<IModelReader> 
ModelFactory::createReader(const std::string& filepath)
{
    std::string ext = getExtension(filepath);

    if (ext == ".stl")
    {
        return std::make_unique<stl::StlReader>();
    }
    else if (ext == ".obj")
    {
        return std::make_unique<obj::ObjReader>();
    }
    else if (ext == ".3mf")
    {
        return std::make_unique<threemf::ThreemfReader>();
    }

    throw std::runtime_error("Unsupported file format: " + ext);
}

core::mesh::Mesh 
ModelFactory::loadModel(const std::string& filepath)
{
    // 1. Uygun reader'ı oluştur
    auto reader = createReader(filepath);

    // 2. Dosyayı oku
    return reader->read(filepath);
}

std::string 
ModelFactory::getExtension(const std::string& filepath)
{
    // Son '.' karakterini bul
    size_t pos = filepath.find_last_of('.');
    
    if (pos == std::string::npos)
    {
        return "";  // Uzantı yok
    }

    // Uzantıyı al
    std::string ext = filepath.substr(pos);

    // Küçük harfe çevir
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return ext;
}

} // namespace models
} // namespace io
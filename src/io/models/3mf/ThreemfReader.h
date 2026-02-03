#pragma once

#include "io/models/common/IModelReader.h"
#include "core/mesh/mesh.h"
#include <string>

namespace io {
namespace models {
namespace threemf {

class ThreemfReader : public IModelReader
{
public:
    ThreemfReader() = default;
    ~ThreemfReader() override = default;

    core::mesh::Mesh read(const std::string& filepath) override;
    bool canRead(const std::string& filepath) const override;
    std::string getFormatName() const override;

private:
    // ZIP'ten XML dosyasını oku
    std::string extractXmlFromZip(const std::string& filepath);

    // XML'den mesh oluştur
    core::mesh::Mesh parseXml(const std::string& xmlContent);
};

} // namespace threemf
} // namespace models
} // namespace io

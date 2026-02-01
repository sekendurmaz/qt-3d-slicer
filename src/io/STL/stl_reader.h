#ifndef STLREADER_H
#define STLREADER_H

#include <string>
#include "mesh/mesh.h"

class STLReader
{
public:
    STLReader() = default;

    bool readSTL(const std::string& filePath, Mesh& mesh);

    const std::string& getLastError() const noexcept { return m_lastError; }

private:
    bool readBinarySTL(const std::string& filePath, Mesh& mesh);
    bool readASCIISTL(const std::string& filePath, Mesh& mesh);
    bool isBinarySTL(const std::string& filePath);

    std::string m_lastError;
};

#endif // STLREADER_H

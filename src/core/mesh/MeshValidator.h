#pragma once

#include "mesh.h"
#include <string>
#include <vector>

namespace core {
namespace mesh {

/**
 * @brief Mesh validation sonuçları
 */
struct ValidationResult
{
    bool isValid = true;                    // Mesh geçerli mi?
    std::vector<std::string> errors;        // Kritik hatalar
    std::vector<std::string> warnings;      // Uyarılar

    // İstatistikler
    int degenerateTriangles = 0;            // Sıfır alanlı üçgenler
    int invalidVertices = 0;                // NaN/Inf koordinatlar
    int duplicateVertices = 0;              // Aynı konumda vertex'ler

    void addError(const std::string& error) {
        errors.push_back(error);
        isValid = false;
    }

    void addWarning(const std::string& warning) {
        warnings.push_back(warning);
    }
};

/**
 * @brief Mesh doğrulama sınıfı
 */
class MeshValidator
{
public:
    MeshValidator() = default;

    /**
     * @brief Mesh'i doğrular
     * @param mesh Doğrulanacak mesh
     * @return ValidationResult
     */
    ValidationResult validate(const Mesh& mesh) const;

    /**
     * @brief Minimum kabul edilebilir üçgen alanı
     */
    void setMinTriangleArea(float area) { minTriangleArea_ = area; }

    /**
     * @brief Vertex'lerin aynı kabul edilmesi için tolerans
     */
    void setVertexTolerance(float tolerance) { vertexTolerance_ = tolerance; }

private:
    float minTriangleArea_ = 1e-6f;         // Minimum triangle area
    float vertexTolerance_ = 1e-5f;         // Vertex equality tolerance

    // Yardımcı fonksiyonlar
    bool isDegenerate(const geometry::Triangle& tri) const;
    bool hasInvalidCoordinates(const geometry::Vec3& v) const;
    float triangleArea(const geometry::Triangle& tri) const;
    bool verticesEqual(const geometry::Vec3& v1, const geometry::Vec3& v2) const;
};

} // namespace mesh
} // namespace core

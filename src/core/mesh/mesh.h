#pragma once

#include <vector>
#include <string>
#include "core/geometry/triangle.h"
#include "core/geometry/aabb.h"

namespace core {
namespace mesh {

class Mesh
{
public:
    std::string name;
    std::vector<geometry::Triangle> triangles;
    geometry::AABB bounds{};

    /**
     * @brief Mesh'i temizle
     */
    void clear() noexcept
    {
        name.clear();
        triangles.clear();
        bounds = geometry::AABB{};
    }

    /**
     * @brief Triangle kapasitesi ayarla (performance critical!)
     *
     * Mesh yüklenirken tahmini triangle sayısını reserve etmek
     * reallocation'ları önler ve yükleme performansını 2x artırır.
     *
     * @param capacity Tahmini triangle sayısı
     */
    void reserve(size_t capacity)
    {
        triangles.reserve(capacity);
    }

    /**
     * @brief Şu anki kapasite
     */
    size_t capacity() const noexcept
    {
        return triangles.capacity();
    }

    /**
     * @brief Fazla belleği serbest bırak
     */
    void shrink_to_fit()
    {
        triangles.shrink_to_fit();
    }

    /**
     * @brief Triangle ekle (copy)
     */
    void addTriangle(const geometry::Triangle& triangle)
    {
        triangles.push_back(triangle);
    }

    /**
     * @brief Triangle ekle (move)
     */
    void addTriangle(geometry::Triangle&& triangle)
    {
        triangles.push_back(std::move(triangle));
    }

    /**
     * @brief 3 vertex + normal ile triangle ekle
     */
    void addTriangle(const geometry::Vec3& v1,
                     const geometry::Vec3& v2,
                     const geometry::Vec3& v3,
                     const geometry::Vec3& normal = {0.0f, 0.0f, 0.0f})
    {
        triangles.emplace_back(v1, v2, v3, normal);
    }

    /**
     * @brief Toplu triangle ekle (batch operation)
     */
    void addTriangles(std::vector<geometry::Triangle>&& tris)
    {
        if (triangles.empty()) {
            triangles = std::move(tris);
        } else {
            triangles.reserve(triangles.size() + tris.size());
            triangles.insert(triangles.end(),
                             std::make_move_iterator(tris.begin()),
                             std::make_move_iterator(tris.end()));
        }
    }

    /**
     * @brief Triangle sayısı
     */
    size_t triangleCount() const noexcept
    {
        return triangles.size();
    }

    /**
     * @brief Mesh boş mu?
     */
    bool isEmpty() const noexcept
    {
        return triangles.empty();
    }

    /**
     * @brief Iterator support (range-based for)
     */
    using iterator = std::vector<geometry::Triangle>::iterator;
    using const_iterator = std::vector<geometry::Triangle>::const_iterator;

    iterator begin() { return triangles.begin(); }
    iterator end() { return triangles.end(); }
    const_iterator begin() const { return triangles.begin(); }
    const_iterator end() const { return triangles.end(); }
    const_iterator cbegin() const { return triangles.cbegin(); }
    const_iterator cend() const { return triangles.cend(); }

    /**
     * @brief AABB hesapla
     */
    bool computeBounds() noexcept;
};

} // namespace mesh
} // namespace core

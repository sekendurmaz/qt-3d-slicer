#pragma once

#include "LineSegment.h"
#include <vector>

namespace core {
namespace slicing {

/**
 * @brief Tek bir slice layer (Z seviyesinde)
 */
class Layer
{
public:
    Layer() = default;
    explicit Layer(float zHeight) : zHeight_(zHeight) {}

    /**
     * @brief Z yüksekliği
     */
    float zHeight() const { return zHeight_; }
    void setZHeight(float z) { zHeight_ = z; }

    /**
     * @brief Line segment ekle
     */
    void addSegment(const LineSegment& segment)
    {
        segments_.push_back(segment);
    }

    /**
     * @brief Tüm segmentler
     */
    const std::vector<LineSegment>& segments() const { return segments_; }

    /**
     * @brief Segment sayısı
     */
    size_t segmentCount() const { return segments_.size(); }

    /**
     * @brief Layer boş mu?
     */
    bool isEmpty() const { return segments_.empty(); }

    /**
     * @brief Temizle
     */
    void clear() { segments_.clear(); }

private:
    float zHeight_ = 0.0f;
    std::vector<LineSegment> segments_;
};

} // namespace slicing
} // namespace core

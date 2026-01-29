#ifndef MESH_H
#define MESH_H

#include <vector>

#include "geometry/triangle.h"
#include "geometry/aabb.h"

struct Mesh
{
    std::vector<Triangle> triangles; // Modelin tüm yüzeyleri
    AABB bounds;                     // Modelin sınır kutusu
};

#endif // MESH_H


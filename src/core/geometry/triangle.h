#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vec3.h"

// STL'deki bir yüzeyi (facet) temsil eder
struct Triangle
{
    Vec3 normal;   // Yüzey normali
    Vec3 vertex1;       // 1. köşe
    Vec3 vertex2;       // 2. köşe
    Vec3 vertex3;       // 3. köşe
};

#endif // TRIANGLE_H

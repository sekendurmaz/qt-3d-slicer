#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vec3.h"

// STL'deki bir yüzeyi (facet) temsil eder
struct Triangle
{
    Vec3 normal;   // Yüzey normali
    Vec3 v1;       // 1. köşe
    Vec3 v2;       // 2. köşe
    Vec3 v3;       // 3. köşe
};

#endif // TRIANGLE_H

#ifndef AABB_H
#define AABB_H

#include "vec3.h"

struct AABB
{
    Vec3 min;   // En küçük köşe
    Vec3 max;   // En büyük köşe
};

#endif // AABB_H

#ifndef VEC3_H
#define VEC3_H

struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;

    Vec3(float x_, float y_, float z_)
        : x(x_), y(y_), z(z_) {}
};

#endif // VEC3_H

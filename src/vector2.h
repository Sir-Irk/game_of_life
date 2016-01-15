#ifndef _vector2_h
#define _vector2_h
#include "math.h"

#if USING_GLM
#include "linux_platform_layer.hpp"
#endif

//#include <math.h>
struct vector2
{
    vector2()
    {
        x = y = 0;
    }

    vector2(real32 _x, real32 _y)
    {
        x = _x;
        y = _y;
    }

    internal real32
    dot(const vector2 &a, const vector2 &b)
    {
        vector2 tempA = a.normalized();
        vector2 tempB = b.normalized();
        return tempA.x * tempB.x + tempA.y * tempB.y;
    }

    internal vector2
    lerp(vector2 start, vector2 end, real32 percent)
    {
        return start + ((end - start) * percent);
    }

    internal vector2
    nlerp(vector2 start, vector2 end, real32 percent)
    {
        return (start + ((end - start) * percent)).normalized();
    }

    internal vector2
    slerp(vector2 start, vector2 end, real32 percent)
    {
        real32 dot = vector2::dot(start, end);
        Clampf(dot, -1.0f, 1.0f);
        real32 theta = acos(dot) * percent;
        vector2 relative = end - start * dot;
        relative.normalize();
        return (start * cos(theta)) + (relative * sin(theta));
    }

    internal inline vector2
    clamp_rect(vector2 current, vector2 min, vector2 max)
    {
        vector2 result;
        result.x = Clampf(current.x, min.x, max.x);
        result.y = Clampf(current.y, min.y, max.y);
        return result;
    }

    internal inline vector2
    rand_in_unit_sphere(real32 scalar)
    {
        vector2 result;
        result.x = (rand() % 100 * ((rand() % 10 > 5) ? 1 : -1));
        result.y = (rand() % 100 * ((rand() % 10 > 5) ? 1 : -1));
        result = result.normalized() * (rand() % 100) * 0.01f;
        return result * scalar;
    }

    real32 length(void) const;
    real32 length_sqr(void) const;
    vector2 normalized(void) const;
    void normalize(void);
    vector2 operator+(const vector2 &a) const;
    vector2 operator-(const vector2 &a) const;
    vector2 operator*(const real32 scalar) const;
    vector2 operator/(const real32 scalar) const;
    vector2 operator-() const;
    void operator+=(const vector2 &other);
    void operator-=(const vector2 &other);
    void operator*=(const real32 scalar);
    void operator/=(const real32 scalar);
    bool32 operator==(const vector2 &other);

#if USING_GLM
    glm::vec3 to_glm_vec3();
    internal vector2
    from_glm_vec3(glm::vec3 vec)
    {
        return vector2(vec.x, vec.y);
    }
#endif

    union
    {
        struct
        {
            real32 x, y;
        };
        real32 v[2];
    };
};

global const vector2 UpAxis(0.0f, -1.0f);
global const vector2 DownAxis(0.0f, 1.0f);
global const vector2 LeftAxis(-1.0f, 0.0f);
global const vector2 RightAxis(1.0f, 0.0f);

#endif

//#include <math.h>
#include "vector2.h"
real32
vector2::length(void) const
{
    return real32(sqrt(this->x * this->x + this->y * this->y));
}
real32
vector2::length_sqr(void) const
{
    return this->x * this->x + this->y * this->y;
}
vector2
vector2::normalized(void) const
{
    return (*this) / length();
}

void
vector2::normalize(void)
{
    (*this) /= length();
}

vector2 vector2::operator+(const vector2 &a) const
{
    vector2 result;
    result.x = this->x + a.x;
    result.y = this->y + a.y;
    return result;
}
vector2 vector2::operator-(const vector2 &a) const
{
    vector2 result;
    result.x = this->x - a.x;
    result.y = this->y - a.y;
    return result;
}
vector2 vector2::operator*(real32 scalar) const
{
    vector2 result;
    result.x = this->x * scalar;
    result.y = this->y * scalar;
    return result;
}
vector2 vector2::operator/(real32 scalar) const
{
    vector2 result;
    result.x = (this->x != 0) ? this->x / scalar : 0;
    result.y = (this->y != 0) ? this->y / scalar : 0;
    return result;
}
vector2 vector2::operator-() const
{
    vector2 result;
    result.x = -this->x;
    result.y = -this->y;
    return result;
}
void vector2::operator+=(const vector2 &other)
{
    this->x += other.x;
    this->y += other.y;
}
void vector2::operator-=(const vector2 &other)
{
    this->x -= other.x;
    this->y -= other.y;
}
void vector2::operator*=(real32 scalar)
{
    this->x *= scalar;
    this->y *= scalar;
}
void vector2::operator/=(real32 scalar)
{
    (this->x != 0) ? this->x /= scalar : this->x = 0;
    (this->y != 0) ? this->y /= scalar : this->y = 0;
}

bool32 vector2::operator==(const vector2 &other)
{
    return (this->x == other.x && this->y == other.y);
}

#if USING_GLM
glm::vec3
vector2::to_glm_vec3()
{
    return glm::vec3(this->x, this->y, 0.0f);
}
#endif

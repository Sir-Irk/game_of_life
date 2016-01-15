#include "AABB.h"

// NOTE: AABB: Axis Aligned Bounding Box

// TODO: make own function to avoid including utility and algorithm
#include <utility>
#include <algorithm>
using std::swap;
using std::max;
using std::min;

AABB AABB::operator+(const vector2 &p) const
{
    AABB result = (*this);
    result.min = p + min;
    result.max = p + max;
    return result;
}

// NOTE: Checks if A contains B
internal bool32
AABB_contains(const AABB &bounds, vector2 point)
{
    if (point.x > bounds.max.x || point.x < bounds.min.x) return false;
    if (point.y > bounds.max.y || point.y < bounds.min.y) return false;
    return true;
}

// NOTE: Checks to see if A contains B
internal bool32
AABB_contains(const AABB &a, const AABB &b)
{
    for (int32 i = 0; i < 2; ++i)
    {
        if (b.min.v[i] < a.min.v[i]) return false;
        if (b.max.v[i] > a.max.v[i]) return false;
    }
    return true;
}

bool32
AABB_intersection(const AABB &a, const AABB &b)
{
    for (int32 i = 0; i < 2; i++)
    {
        if (a.min.v[i] > b.max.v[i]) return false;
        if (a.max.v[i] < b.min.v[i]) return false;
    }
    return true;
}

bool32
AABB_clip_line(
    int32 d, const AABB &aabbBox, const vector2 &v0, const vector2 &v1, real32 &low, real32 high)
{
    real32 dimLow = (aabbBox.min.v[d] - v0.v[d]) / (v1.v[d] - v0.v[d]);
    real32 dimHigh = (aabbBox.max.v[d] - v0.v[d]) / (v1.v[d] - v0.v[d]);
    if (dimHigh < dimLow) swap(dimHigh, dimLow);
    if (dimHigh < low) return false;
    if (dimLow > high) return false;

    low = max(dimLow, low);
    high = min(dimHigh, high);

    if (low > high) return false;

    return true;
}

bool32
AABB_line_intersection(
    const AABB &aabbBox, const vector2 &v0, const vector2 &v1, vector2 &intersection, real32 &fraction)
{
    real32 low = 0;
    real32 high = 1;

    if (!AABB_clip_line(0, aabbBox, v0, v1, low, high)) return false;
    if (!AABB_clip_line(1, aabbBox, v0, v1, low, high)) return false;

    vector2 b = v1 - v0;
    intersection = v0 + b * low;
    fraction = low;
    return true;
}

bool32
AABB_trace_line(const AABB *box, vector2 targetPos, vector2 v0, vector2 v1, vector2 *intersection)
{
    real32 lowestFraction = 1.0f;
    real32 testFraction;
    vector2 testInter;

    if (AABB_line_intersection(*box + targetPos, v0, v1, testInter, testFraction) &&
        testFraction < lowestFraction)
    {
        *intersection = testInter;
        lowestFraction = testFraction;
    }

    if (lowestFraction < 1) return true;

    return false;
}

void
AABB_Rotate90(AABB *box)
{
    box->min = vector2(-box->min.y, box->min.x);
    box->max = vector2(-box->max.y, box->max.x);
    AABB_Recanonicalize(box);
}

// NOTE Ensures that min and max are in correct positions(useful after making transformations)
void
AABB_Recanonicalize(AABB *box)
{
    box->min = vector2(-ABSOLUTE(box->min.x), -ABSOLUTE(box->min.y));
    box->max = vector2(ABSOLUTE(box->max.x), ABSOLUTE(box->max.y));
}

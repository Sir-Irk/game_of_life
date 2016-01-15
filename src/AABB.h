#ifndef _AABB_h
#define _AABB_h

// NOTE: AABB: Axis Aligned Bounding Box

#include "vector2.h"
struct AABB
{
    vector2 min;
    vector2 max;
    AABB operator+(const vector2 &p) const;
};

bool32 AABB_intersection(const AABB &a, const AABB &b);

// NOTE: Checks if "bounds" contains "point"
internal bool32 AABB_contains(const AABB &bounds, vector2 point);

// NOTE: Checks to see if "a" contains "b"
internal bool32 AABB_contains(const AABB &a, const AABB &b);

bool32 AABB_clip_line(
    int d, const AABB &aabbBox, const vector2 &v0, const vector2 &v1, real32 &low, real32 high);

bool32 AABB_line_intersection(
    const AABB &aabbBox, const vector2 &v0, const vector2 &v1, vector2 &intersection, real32 &fraction);

bool32 AABB_trace_line(const AABB *box, vector2 targetPos, vector2 v0, vector2 v1, vector2 *intersection);
void AABB_Rotate90(AABB *box);
void AABB_Recanonicalize(AABB *box);
#endif

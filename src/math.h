#ifndef _math_h
#define _math_h

#include "types.h"
#include <math.h>

#define ABSOLUTE(value) (((value) >= 0) ? (value) : -(value))
#define MIN(x, y) ((x < y) ? x : y)
#define MAX(x, y) ((x > y) ? x : y)

inline real32
Approach(real32 goal, real32 current, real32 deltaTime)
{
    real32 difference = goal - current;
    if (difference > deltaTime)
    {
        return current + deltaTime;
    }
    if (difference < -deltaTime)
    {
        return current - deltaTime;
    }
    return goal;
}

inline real32
Clampf(real32 current, real32 min, real32 max)
{
    if (current >= max) return max;
    if (current <= min) return min;
    return current;
}

// TODO: Fix this round function. Seems to give inconsistent results
inline int32
Roundf(real32 num)
{
    // TODO: replace this with better round.
    int32 truncation = (int32)num;
    return (num - truncation >= 0.5f) ? truncation + 1 : truncation;
}

inline real32
SquareRoot(real32 num)
{
    real32 Result = sqrt(num);
    return Result;
}

inline real32
AbsReal32(real32 value)
{
    return ABSOLUTE(value);
}

inline int32
Mod(int32 x, int32 factor)
{
    int32 result = x % factor;
    if (result < 0) result += factor;
    return result;
}

inline int32
RoundMod(real32 x, int32 mod)
{
    int32 result = x;
    // TODO: Switch truncation to rounding after round function is fixed
    // int32 modded = Mod(Roundf(x), mod);
    int32 modded = Mod(x, mod);
    result += (modded >= mod / 2) ? (mod - modded) : -modded;
    return result;
}

inline real32
rotation_clamp(real32 rot)
{
	real32 result = rot;
    if (rot > 360.0f)
        result -= 360.0f;
    else if (rot < 0.0f)
        result += 360.0f;
	return result;
}
#endif

#ifndef _linux_platform_layer_hpp
#define _linux_platform_layer_hpp

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// NOTE:Graphics(openGL):
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// NOTE: Audio(openAL)
#include <AL/al.h>
#include <AL/alc.h>

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#include "types.h"
#include "vector2.h"
#include "audio.h"
#include "math.h"
#include "render.h"

#define BIT_IS_SET(mask, bit) (((mask) >> (bit)) & 1)

struct game_state;
struct input_state;
struct key_info;
struct mouse_info;
struct debug_read_file_result;
struct memory_arena;

global const real32 g_fixedDeltaTime = 0.08f;
global uint32 newTime;

struct memory_arena
{
    mem_size size;
    mem_size used;
    uint8 *base;
};

struct key_info
{
    bool32 up;
    bool32 down;
    bool32 left;
    bool32 right;
    bool32 hire;
    bool32 escape;

    bool32 modCtrl;
    bool32 modAlt;
    bool32 modShift;

    bool32 numKey1;
    bool32 numKey2;
    bool32 numKey3;
    bool32 numKey4;

    void
    Reset(void)
    {
        up = down = left = right = escape = hire = 0;
    }
};

struct mouse_info
{
    vector2 position;
    vector2 worldPosition;
    uint32 buttonMask;
    uint32 justPressedMask;
};

struct input_state
{
    key_info keys;
    mouse_info mouse;
};

struct debug_read_file_result
{
    uint32 ContentsSize;
    void *Contents;
};

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(char *filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_read_entire_file);
DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFileStub)
{
    debug_read_file_result result = {};
    return result;
}

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name)                                                          \
    bool32 name(char *filename, uint32 memorySize, void *memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_write_entire_file);
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFileStub)
{
    return (false);
}

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_free_file_memory);
DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemoryStub)
{
}

#include "game.h"

#endif

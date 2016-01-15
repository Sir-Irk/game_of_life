#ifndef _game_h
#define _game_h
#include "linux_platform_layer.hpp"
#include "types.h"
#include "vector2.h"
#include "render.h"
#include "AABB.h"

#define GAME_LOG_FILE "game.log"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1200

global real32 g_frameTime_accumulator;

struct game_memory
{
    mem_size permanentStorageSize;
    void *permanentStorage;
    mem_size transientStorageSize;
    void *transientStorage;

    debug_read_entire_file *debugReadEntireFile;
    debug_write_entire_file *debugWriteEntireFile;
    debug_free_file_memory *debugFreeFileMemory;

    bool32 initialized;
};

struct cell
{
    vector2 pos; // NOTE: Probably don't need;
};

struct cell_state_array
{
	bool32 *ptr;
	int32 size;
};

#define NEIGHBOR_COUNT 8
global const int32 g_neighbor_grid[8][2] = 
{
	{-1, -1}, {0, -1}, {1, -1},
	{-1,  0},          {1,  0},
	{-1,  1}, {0,  1}, {1,  1}
};

struct game_state
{
    memory_arena worldArena;
    input_state input;
    Renderer::vertex_array_data cellVerts;
	uint32 aliveTexture;
	uint32 deadTexture;
	int32 cellWidth;

	cell_state_array cellStatesA;
	cell_state_array cellStatesB;
	cell_state_array *currentCellState;

	int32 *nCounts;

	AABB cellBounds;

	real32 timer;
	real32 delay;

    glm::vec3 viewPos;
    glm::mat4 viewMat;
    GLint viewUni;
};

#define GAME_INITIALIZATION(name)                                                                       \
    bool32 name(GLFWwindow *window, game_memory *gameMemory, game_state **gameState)
typedef GAME_INITIALIZATION(game_initialization);
GAME_INITIALIZATION(GameInitializationStub)
{
    return (true);
}

#define GAME_INITIALIZE_RENDERER(name)                                                                  \
    bool32 name(GLFWwindow *window, game_memory *gameMemory, game_state *gameState)
typedef GAME_INITIALIZE_RENDERER(game_initialize_renderer);
GAME_INITIALIZE_RENDERER(GameInitializeRendererStub)
{
    return (true);
}

#define GAME_UPDATE_AND_RENDER(name)                                                                    \
    void name(GLFWwindow *window, game_memory *gameMemory, game_state **gameState,                      \
              real32 fixedDeltaTime, real32 frameTime)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub)
{
}

#define FREE_GAME_MEMORY(name) void name(game_memory *gameMemory, game_state *game_state)
typedef FREE_GAME_MEMORY(free_game_memory);
FREE_GAME_MEMORY(FreeGameMemoryStub)
{
}

#define PushStruct(arena, type) (type *) push_size_(arena, sizeof(type))
#define PushArray(arena, count, type) (type *) push_size_(arena, (count) * sizeof(type))
internal void *push_size_(memory_arena *arena, mem_size size);
internal void initialize_arena(memory_arena *arena, mem_size size, uint8 *base);
internal bool32 game_initialize_memory(game_state **gameState, game_memory *gameMemory);
internal bool32 start_new_log(char *filename);
internal void log_to_console(char *filename, const char *message, va_list argptr);
internal bool32 log_to_file(char *filename, const char *message, va_list argptr);
internal bool32 get_cell_state(cell_state_array *cell, int32 x, int32 y, int32 width, int32 height);
#endif

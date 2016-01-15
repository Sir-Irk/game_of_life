//#include "game.h"
#include "render.cpp"
#include "vector2.cpp"
#include "AABB.cpp"
#include "audio.cpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/gtx/rotate_vector.hpp>

internal void *
push_size_(memory_arena *arena, mem_size size)
{
    assert((arena->used + size) <= arena->size);
    void *result = arena->base + arena->used;
    arena->used += size;
    return result;
}

internal void
initialize_arena(memory_arena *arena, mem_size size, uint8 *base)
{
    arena->size = size;
    arena->base = base;
    arena->used = 0;
}

internal bool32
start_new_log(char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "ERROR: could not open log file %s for writing\n", filename);
        return false;
    }

    time_t now = time(NULL);
    char *date = ctime(&now);
    fprintf(file, "%s: local time %s\n", filename, date);
    fclose(file);
    return true;
}

internal void
log_to_console(char *filename, const char *message, va_list argptr)
{
    vfprintf(stderr, message, argptr);
}

internal bool32
log_to_file(char *filename, const char *message, va_list argptr)
{
    FILE *file = fopen(filename, "a");
    if (!file)
    {
        fprintf(stderr, "ERROR: could not open %s for appending\n", filename);
        return false;
    }

    vfprintf(file, message, argptr);
    fclose(file);
    return true;
}

internal bool32
game_log(bool32 error, const char *message, ...)
{
    va_list argptr;
    va_start(argptr, message);
    log_to_file(GAME_LOG_FILE, message, argptr);
    va_end(argptr);
    if (error)
    {
        va_start(argptr, message);
        log_to_console(GAME_LOG_FILE, message, argptr);
        va_end(argptr);
    }
    return true;
}

internal uint32
DEBUGLoadBitmap(char *filename, debug_read_entire_file *readEntireFile)
{
    uint32 result = 0;
    int32 x, y, n;
    int32 force_channels = 4;
    uint8 *pixels = stbi_load(filename, &x, &y, &n, force_channels);
    assert(pixels);

    bitmap_header header = {};
    header.width = x;
    header.height = y;
    int64 size = (header.width * header.height) * 4;

    return gl_load_bitmap_texture(&header, pixels);
}

global GLuint g_floorTexID = 0;

extern "C" GAME_INITIALIZE_RENDERER(GameInitializeRenderer)
{
    assert(window && gameMemory && gameState);
    if (!Renderer::initialize_opengl(window))
    {
        game_log(true, "\nfailed to initialize opengl\n");
        return false;
    }

    game_state *gs = gameState;
    debug_read_entire_file *readFile = gameMemory->debugReadEntireFile;

    color3 cellColor = {1, 1, 1};
    gs->cellBounds.min = vector2(-(gs->cellWidth / 2), -(gs->cellWidth / 2));
    gs->cellBounds.max = -gs->cellBounds.min;
    Renderer::gl_create_quad(&gs->worldArena, &gs->cellVerts, cellColor, gs->cellBounds);
    gs->aliveTexture = DEBUGLoadBitmap("../textures/alive_01.tga", readFile);
    gs->deadTexture = DEBUGLoadBitmap("../textures/dead_01.tga", readFile);

    // NOTE: initialize view(camera)
    gs->viewUni = glGetUniformLocation(g_shader_program, "g_view");
    glUniformMatrix4fv(gs->viewUni, 1, GL_FALSE, glm::value_ptr(gs->viewMat));

    return true;
}

extern "C" GAME_INITIALIZATION(GameInitialization)
{
    if (!start_new_log(GAME_LOG_FILE)) fprintf(stderr, "Failed to start new game log\n");
    if (!game_initialize_memory(gameState, gameMemory))
        game_log(true, "Fatal: Failed to initialize memory\n");

    game_state *gs = *gameState;

    gs->cellWidth = 8;
    int32 maxX = (SCREEN_WIDTH / gs->cellWidth) - 1;
    int32 maxY = SCREEN_HEIGHT / gs->cellWidth;
    gs->cellStatesA.size = gs->cellStatesB.size = maxX * maxY;
    assert(gs->cellStatesA.ptr = PushArray(&gs->worldArena, gs->cellStatesA.size, bool32));
    assert(gs->cellStatesB.ptr = PushArray(&gs->worldArena, gs->cellStatesB.size, bool32));
    assert(gs->nCounts = PushArray(&gs->worldArena, gs->cellStatesA.size, int32));

    gs->currentCellState = &gs->cellStatesA;

    gs->cellStatesA.ptr[maxX * 38 + 38] = true;
    gs->cellStatesA.ptr[maxX * 38 + 37] = true;
    gs->cellStatesA.ptr[maxX * 37 + 38] = true;
    gs->cellStatesA.ptr[maxX * 37 + 39] = true;
    gs->cellStatesA.ptr[maxX * 39 + 38] = true;

    gs->cellStatesA.ptr[maxX * 138 + 138] = true;
    gs->cellStatesA.ptr[maxX * 138 + 137] = true;
    gs->cellStatesA.ptr[maxX * 137 + 138] = true;
    gs->cellStatesA.ptr[maxX * 137 + 139] = true;
    gs->cellStatesA.ptr[maxX * 139 + 138] = true;

    memcpy(gs->cellStatesB.ptr, gs->cellStatesA.ptr, maxX * maxY * sizeof(bool32));

    // NOTE: Precalculate Neighbor count
    for (int32 y = 0; y < maxY; ++y)
    {
        for (int32 x = 0; x < maxX; ++x)
        {
            int32 index = maxX * y + x;
            int32 nCount = 0;
            for (int32 i = 0; i < NEIGHBOR_COUNT; ++i)
            {
                int32 curX = x + g_neighbor_grid[i][0];
                int32 curY = y + g_neighbor_grid[i][1];
                if (get_cell_state(&gs->cellStatesA, curX, curY, maxX, maxY)) ++nCount;
            }

            gs->nCounts[index] = nCount;
        }
    }

    if (!GameInitializeRenderer(window, gameMemory, gs))
    {
        game_log(true, "failed to initialize renderer\n");
        return false;
    }

    return true;
}

internal bool32
game_initialize_memory(game_state **gameState, game_memory *gameMemory)
{
    gameMemory->permanentStorageSize = Megabytes(32);
    gameMemory->permanentStorage = (void *)calloc(1, gameMemory->permanentStorageSize);

    gameMemory->transientStorageSize = Megabytes(32);
    gameMemory->transientStorage = (void *)calloc(1, gameMemory->transientStorageSize);
    if (!gameMemory->transientStorage || !gameMemory->permanentStorage)
    {
        game_log(true, "game memory failed to alloc\n");
        return false;
    }

    *gameState = (game_state *)gameMemory->permanentStorage;
    game_state *gs = *gameState;
    initialize_arena(&gs->worldArena, gameMemory->permanentStorageSize - sizeof(game_state),
                     (uint8 *)gameMemory->permanentStorage + sizeof(game_state));

    return true;
}

internal bool32
play_sound(uint32 sourceID)
{
    alSourcePlay(sourceID);
    return true;
}

internal bool32
is_valid_cell(cell_state_array *cell, int32 x, int32 y, int32 width, int32 height)
{
    return !(x > width - 1 || y > height - 1 || x < 0 || y < 0);
}

internal bool32
get_cell_state(cell_state_array *cell, int32 x, int32 y, int32 width, int32 height)
{
    if (!is_valid_cell(cell, x, y, width, height)) return false;
    int32 index = width * y + x;
    if (index < 0 || (index > (width * height - 1))) return false;
    return cell->ptr[index];
}

internal void
set_cell(
    bool32 value, cell_state_array *cells, int32 *nCounts, int32 x, int32 y, int32 width, int32 height)
{
	int32 index = width * y + x;
	if(cells->ptr[index] == value) return;

	cells->ptr[index] = value;
	if(!nCounts) return;

	int32 increment = (value ? 1 : -1);

	for(int32 i = 0; i < NEIGHBOR_COUNT; ++i)
	{
		int32 curX = x + g_neighbor_grid[i][0];	
		int32 curY = y + g_neighbor_grid[i][1];	
		if(is_valid_cell(cells, curX, curY, width, height))
		{
			int32 curIndex = width * curY + curX;
			nCounts[curIndex] += increment;
			if(nCounts[curIndex] < 0) nCounts[curIndex] = 0;
		}
	}
}

internal void
update_cells(cell_state_array *cur, cell_state_array *succ, int32 *nCounts, int32 width, int32 height)
{
    for (int32 y = 0; y < height; ++y)
    {
        for (int32 x = 0; x < width; ++x)
        {
            int32 index = width * y + x;
            bool32 *curCell = &cur->ptr[index];
            bool32 *succCell = &succ->ptr[index];

            if (!(*curCell) && !nCounts[index])
			{
				*succCell = *curCell;
				continue;
			}

            int32 nCount = 0;
            for (int32 i = 0; i < NEIGHBOR_COUNT; ++i)
            {
                int32 curX = x + g_neighbor_grid[i][0];
                int32 curY = y + g_neighbor_grid[i][1];
                if (get_cell_state(cur, curX, curY, width, height)) ++nCount;
            }

            if (nCount < 2 && *curCell)
                set_cell(false, succ, nCounts, x, y, width, height); 
            else if (nCount > 3 && *curCell)
                set_cell(false, succ, nCounts, x, y, width, height); 
            else if (nCount == 3 && !(*curCell))
                set_cell(true, succ, nCounts, x, y, width, height); 
            else
                set_cell(*curCell, succ, nCounts, x, y, width, height); 
        }
    }
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    *gameState = (game_state *)gameMemory->permanentStorage;
    game_state *gs = *gameState;
    mouse_info *mouse = &gs->input.mouse;
    key_info *keys = &gs->input.keys;

    int32 maxX = (SCREEN_WIDTH / gs->cellWidth) - 1;
    int32 maxY = SCREEN_HEIGHT / gs->cellWidth;

    if ((gs->timer += fixedDeltaTime) > gs->delay)
    {
        cell_state_array *otherCells = NULL;
        if (gs->currentCellState == &gs->cellStatesA)
        {
            gs->currentCellState = &gs->cellStatesB;
            otherCells = &gs->cellStatesA;
        }
        else if (gs->currentCellState == &gs->cellStatesB)
        {
            gs->currentCellState = &gs->cellStatesA;
            otherCells = &gs->cellStatesB;
        }
        update_cells(gs->currentCellState, otherCells, gs->nCounts, maxX, maxY);

        gs->timer = 0.0f;
    }

#define DELAY_INCREMENT_AMOUNT 0.001f

    if (keys->up) gs->delay += DELAY_INCREMENT_AMOUNT;
    if (keys->down && gs->delay - DELAY_INCREMENT_AMOUNT >= 0) gs->delay -= DELAY_INCREMENT_AMOUNT;

    bool32 mouseButtonPressed = BIT_IS_SET(mouse->buttonMask, GLFW_MOUSE_BUTTON_1);

    gs->viewMat = glm::translate(glm::mat4(), gs->viewPos);
    glUniformMatrix4fv(gs->viewUni, 1, GL_FALSE, glm::value_ptr(gs->viewMat));

    int32 xPos = 0, yPos = 0;
    for (int32 y = 0; y < maxY; ++y)
    {
        for (int32 x = 0; x < maxX; ++x)
        {
            uint32 tex =
                ((gs->currentCellState->ptr[y * maxX + x]) ? gs->aliveTexture : gs->deadTexture);
            xPos = (gs->cellWidth * x) + gs->cellWidth / 2;
            yPos = (gs->cellWidth * y) + gs->cellWidth / 2;

            if (mouseButtonPressed)
            {
                mouse->worldPosition = mouse->position - vector2::from_glm_vec3(gs->viewPos);
                if (AABB_contains(gs->cellBounds + vector2(xPos, yPos), mouse->worldPosition))
                {
                    gs->cellStatesA.ptr[maxX * y + x] = true;
                    gs->cellStatesB.ptr[maxX * y + x] = true;
                    for (int32 i = 0; i < NEIGHBOR_COUNT; ++i)
                    {
                        int32 curX = x + g_neighbor_grid[i][0];
                        int32 curY = y + g_neighbor_grid[i][1];
                        if (is_valid_cell(gs->currentCellState, curX, curY, maxX, maxY))
                        {
							set_cell(true, &gs->cellStatesA, gs->nCounts, curX, curY, maxX, maxY);
							set_cell(true, &gs->cellStatesA, NULL, curX, curY, maxX, maxY);
                        }
                    }
                }
            }

            Renderer::gl_draw_textured(gs->cellVerts.vao, glm::vec3(xPos, yPos, 0), tex);
        }
    }

    Renderer::gl_draw(window);
}

extern "C" FREE_GAME_MEMORY(FreeGameMemory)
{
    if (gameMemory->permanentStorage) free(gameMemory->permanentStorage);
    gameMemory->permanentStorage = NULL;
    if (gameMemory->transientStorage) free(gameMemory->transientStorage);
    gameMemory->transientStorage = NULL;
    Renderer::gl_cleanup();
    if (g_audioDevice) alcCloseDevice(g_audioDevice);
    if (g_audioContext) alcDestroyContext(g_audioContext);
}
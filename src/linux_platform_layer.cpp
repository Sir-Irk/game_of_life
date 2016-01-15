// NOTE: Monolithic build
#include "linux_platform_layer.hpp"
//#include "vector2.cpp"
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

//#include "audio.cpp"

global game_state *g_gameState;
global game_memory g_gameMemory;
static int32 running = 1;

internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
internal bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize, void *Memory);
internal void DEBUGPlatformFreeFileMemory(void *Memory);

struct platform_game_code
{
#define GAME_DLL_FILENAME "game.so"
#define GAME_TEMP_DLL_FILENAME "game_temp.so"
#define GAME_TEMP_2_DLL_FILENAME "game_temp_2.so"

    void *gameDLL;
    game_initialization *gameInitialization;
    game_initialize_renderer *gameInitializeRenderer;
    game_update_and_render *gameUpdateAndRender;
    free_game_memory *freeGameMemory;
    bool32 isValid;
    bool32 useTemp;
};

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};
    FILE *handle = fopen(filename, "r");
    struct stat statbuf;
    off_t filesize = 0;
    if (handle)
    {
        if (stat(filename, &statbuf) >= 0)
        {
            filesize = statbuf.st_size;
            if ((Result.Contents = (void *)malloc(filesize)))
            {
                if (!fread(Result.Contents, filesize, 1, handle))
                {
                    fprintf(stderr, "Read failed\n");
                    DEBUGPlatformFreeFileMemory(Result.Contents);
                    Result.Contents = NULL;
                }
                else
                {
                    Result.ContentsSize = filesize;
                }
            }
            else
            {
                fprintf(stderr, "Malloc failed\n");
            }
        }
        else
        {
            fprintf(stderr, "File Stat Failed\n");
        }
        fclose(handle);
    }
    return Result;
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if (memory) free(memory);
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    bool32 result = false;
    if (!memory) return result;
    FILE *handle = fopen(filename, "w+");
    if (handle)
    {
        if (fwrite(memory, memorySize, 1, handle))
        {
            result = true;
        }
        else
        {
            fprintf(stderr, "Failed to write to file\n");
        }
        fclose(handle);
    }
    else
    {
        fprintf(stderr, "Failed to open/create file\n");
    }
    return result;
}

internal bool32
copy_file(char *src, char *dest)
{
    FILE *inFile = fopen(src, "rb");
    if (!inFile)
    {
        fprintf(stderr, "Failed to load in-file for copy: %s\n", src);
        perror(NULL);
        return false;
    }
    FILE *outFile = fopen(dest, "w+b");
    if (!outFile)
    {
        fprintf(stderr, "Failed to load out-file for copy: %s\n", dest);
        perror(NULL);
        return false;
    }
    // NOTE: May need a larger buffer eventually;
    char buffer[200000];
    mem_size bytes;
    bytes = fread(buffer, 1, sizeof(buffer), inFile);
    fwrite(buffer, 1, bytes, outFile);
    fclose(inFile);
    fclose(outFile);
    return true;
}

inline int64
strlen(char *s)
{
    int64 result = 0;
    if (!s) return result;
    for (; s[result] != '\0'; ++result)
        ;
    return result;
}

internal platform_game_code
platform_load_game_code(char *dllName)
{
    platform_game_code result = {};

    char *cwd = getcwd(NULL, 0);
    char *path = NULL;
    if (cwd)
    {
        int32 cwdLen = strlen(cwd) + 1;
        int32 dllNameLen = strlen(dllName);
        int32 len = dllNameLen + cwdLen + 1;
        path = (char *)malloc(len);
        if (path)
        {
            memcpy(path, cwd, cwdLen);
            path[cwdLen - 1] = '/';
            memcpy((path + cwdLen), dllName, dllNameLen);
            path[len - 1] = '\0';
        }
    }

    if (path)
        result.gameDLL = dlopen(path, RTLD_NOW);
    else
    {
        fprintf(stderr, "Failed to resolve pathname\n");
        result.isValid = false;
        return result;
    }

    if (result.gameDLL)
    {
        result.isValid = true;
        result.gameInitialization = (game_initialization *)dlsym(result.gameDLL, "GameInitialization");
        if (!result.gameInitialization)
        {
            fprintf(stderr, "failed to load game_initialization\n");
            result.isValid = false;
        }

        result.gameInitializeRenderer =
            (game_initialize_renderer *)dlsym(result.gameDLL, "GameInitializeRenderer");
        if (!result.gameInitializeRenderer)
        {
            fprintf(stderr, "failed to load game_initialize_renderer\n");
            result.isValid = false;
        }

        result.gameUpdateAndRender =
            (game_update_and_render *)dlsym(result.gameDLL, "GameUpdateAndRender");
        if (!result.gameUpdateAndRender)
        {
            fprintf(stderr, "failed to load game_update_and_render\n");
            result.isValid = false;
        }

        result.freeGameMemory = (free_game_memory *)dlsym(result.gameDLL, "FreeGameMemory");
        if (!result.freeGameMemory)
        {
            fprintf(stderr, "failed to load freeGameMemory\n");
            result.isValid = false;
        }
    }
    else
    {
        char *error = dlerror();
        result.isValid = false;
        fprintf(stderr, "failed to load game.so: %s\n", (error) ? error : "no error from dlerror()");
    }

    if (!result.isValid)
    {
        result.gameInitialization = GameInitializationStub;
        result.gameInitializeRenderer = GameInitializeRendererStub;
        result.gameUpdateAndRender = GameUpdateAndRenderStub;
        result.freeGameMemory = FreeGameMemoryStub;
    }

    if (path) free(path);
    if (cwd) free(cwd);

    return result;
}

internal void
platform_unload_game_code(platform_game_code *gameCode)
{
    if (gameCode->gameDLL)
    {
        dlclose(gameCode->gameDLL);
        gameCode->gameDLL = NULL;
    }
    gameCode->gameInitialization = NULL;
    gameCode->gameUpdateAndRender = NULL;
    gameCode->freeGameMemory = NULL;
    gameCode->isValid = false;
}

internal bool32
platform_reload_game_code(platform_game_code *gameCode)
{
    char *tempName;

    tempName = (gameCode->useTemp ? (char *)GAME_TEMP_2_DLL_FILENAME : (char *)GAME_TEMP_DLL_FILENAME);

    if (copy_file(GAME_DLL_FILENAME, tempName))
    {
        platform_unload_game_code(gameCode);
        bool32 useTemp = gameCode->useTemp;
        *gameCode = platform_load_game_code(tempName);
        gameCode->useTemp = !useTemp;
        printf("Game code %s\n", (gameCode->isValid ? "reloaded successfully" : "failed to reload"));
        return true;
    }

    printf("Game code failed to reload\n");
    return false;
}

// TODO: Remove this global by using better key handling function(instead of this callback)

internal void
key_callback(GLFWwindow *window, int32 key, int32 scancode, int32 action, int32 mods)
{
    input_state *input = (input_state *)glfwGetWindowUserPointer(window);
    key_info *k = &input->keys;
    switch (action)
    {
        case GLFW_PRESS:
        case GLFW_RELEASE:
        {
            bool32 wasDown = action == GLFW_RELEASE;
            switch (key)
            {
                case GLFW_KEY_ESCAPE:
                {
                    k->escape = !wasDown;
                }
                break;
                case GLFW_KEY_W:
                {
                    k->up = !wasDown;
                }
                break;
                case GLFW_KEY_A:
                {
                    k->left = !wasDown;
                }
                break;
                case GLFW_KEY_S:
                {
                    k->down = !wasDown;
                }
                break;
                case GLFW_KEY_D:
                {
                    k->right = !wasDown;
                }
                break;
                case GLFW_KEY_SPACE:
                {
                    k->hire = !wasDown;
                }
                break;
                case GLFW_KEY_1:
                {
                    k->numKey1 = !wasDown;
                }
                break;
                case GLFW_KEY_2:
                {
                    k->numKey2 = !wasDown;
                }
                break;
                case GLFW_KEY_3:
                {
                    k->numKey3 = !wasDown;
                }
                break;
                case GLFW_KEY_4:
                {
                    k->numKey4 = !wasDown;
                }
                break;
            }
        }
        break;
    }
}

internal void
mouse_position_callback(GLFWwindow *window, double x, double y)
{
    input_state *input = (input_state *)glfwGetWindowUserPointer(window);
    input->mouse.position.x = x;
    input->mouse.position.y = y;
}

internal void
mouse_button_callback(GLFWwindow *window, int32 button, int32 action, int32 mods)
{
    input_state *input = (input_state *)glfwGetWindowUserPointer(window);
    uint32 mask = input->mouse.buttonMask;
    input->mouse.buttonMask ^= (-action ^ mask) & (1 << button);
    input->mouse.justPressedMask = input->mouse.buttonMask;
}

#if GAME_DEBUG
int
main(int argc, char **argv)
#else
int
main(void)
#endif
{
	assert(copy_file(GAME_DLL_FILENAME, GAME_TEMP_DLL_FILENAME));
	assert(copy_file(GAME_DLL_FILENAME, GAME_TEMP_2_DLL_FILENAME));
    platform_game_code gameCode = platform_load_game_code("game_temp.so");
    gameCode.useTemp = true;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 16);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);

#if GAME_DEBUG
    int32 numMonitors = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&numMonitors);
    if (numMonitors < 2)
    {
        fprintf(stderr, "failed to find secondary monitor");
        return 1;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(monitors[1]);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_FLOATING, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "ProCo", monitors[1], NULL);
#else
    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ProCo", NULL, NULL);
#endif

    if (!window)
    {
        printf("failed to create window\n");
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    game_memory gameMemory = {};
    gameMemory.debugReadEntireFile = DEBUGPlatformReadEntireFile;
    gameMemory.debugWriteEntireFile = DEBUGPlatformWriteEntireFile;
    gameMemory.debugFreeFileMemory = DEBUGPlatformFreeFileMemory;

    game_state *gameState = NULL;
    if (window == NULL)
    {
        fprintf(stderr, "ERROR: Could not create window: \n");
        return 0;
    }

    if (!gameCode.gameInitialization(window, &gameMemory, &gameState))
    {
        fprintf(stderr, "Failed to initialize game\n");
        return 0;
    }

    uint32 prevTime = 0;
    newTime = glfwGetTime();

    glfwSetWindowUserPointer(window, &gameState->input);

    int32 gameCodeReloadTimer = 0;
#if GAME_DEBUG
    printf("Initialization successful. Starting main loop\n");
#endif
    while (!glfwWindowShouldClose(window))
    {
        newTime = glfwGetTime();
        real32 frameTime = (newTime - prevTime);
        prevTime = newTime;

#if GAME_DEBUG
        ++gameCodeReloadTimer;

        if (gameCodeReloadTimer >= 300)
        {
            if (platform_reload_game_code(&gameCode))
            {
                if (!gameCode.gameInitializeRenderer(window, &gameMemory, gameState))
                {
                    printf("Failed to initialize renderer\n");
                    return 0;
                }
            }

            gameCodeReloadTimer = 0;
        }
#endif
        glfwPollEvents();
        if (gameState->input.keys.escape) break;
        gameCode.gameUpdateAndRender(window, &gameMemory, &gameState, g_fixedDeltaTime, frameTime);
        gameState->input.mouse.justPressedMask = 0;
    }
    gameCode.freeGameMemory(&g_gameMemory, gameState);
    glfwTerminate();

    return 0;
}

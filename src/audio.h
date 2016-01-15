#ifndef _audio_h
#define _audio_h

#include "linux_platform_layer.hpp"
#include <AL/alut.h>

#define AL_LOG_FILE "al.log"

global ALCdevice *g_audioDevice = NULL;
global ALCcontext *g_audioContext = NULL;

global ALfloat g_listenerPos[] = {0.0f, 0.0f, 0.0f};
global ALfloat g_listenerVel[] = {0.0f, 0.0f, 0.0f};
global ALfloat g_listenerOri[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};

global ALfloat g_audioSourcePos[] = {-2.0f, 0.0f, 0.0f};
global ALfloat g_audioSourceVel[] = {0.0f, 0.0f, 0.0f};

#define MAX_SOUNDS 16

#define AL_NUM_BUFFERS MAX_SOUNDS
#define AL_NUM_SOURCES MAX_SOUNDS
#define AL_NUM_ENV 1

struct sound_array
{
    uint32 *sources;
    uint32 *buffers;
    int32 next;
    int32 size;
};

global int32 g_loadedSoundsIndex;
global ALuint g_audioEnvironments[AL_NUM_ENV];

internal bool32 initialize_sound(sound_array *sounds);
internal bool32 al_log(bool32 error, const char *message, ...);
internal bool32 handle_alGetError();
internal bool32 al_print_if_error(char *message);
internal uint32 al_add_sound(char *filename);
internal bool32 al_load_sound_file(uint32 *sourceID, uint32 *bufferID, char *filename);

#endif

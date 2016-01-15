#include "audio.h"

#include <AL/alure.h>
internal bool32
handle_alGetError()
{
    bool32 hadError = false;
    ALenum error = AL_NO_ERROR;
    while ((error = alGetError()) != AL_NO_ERROR)
    {
        switch (error)
        {
            case AL_INVALID_NAME:
            {
                hadError = true;
                al_log(true, "OpenAL: Invalid name\n");
            }
            break;
            case AL_INVALID_ENUM:
            {
                hadError = true;
                al_log(true, "OpenAL: Invalid enum\n");
            }
            break;
            case AL_INVALID_VALUE:
            {
                hadError = true;
                al_log(true, "OpenAL: Invalid value\n");
            }
            break;
            case AL_INVALID_OPERATION:
            {
                hadError = true;
                al_log(true, "OpenAL: Invalid operation\n");
            }
            break;
            case AL_OUT_OF_MEMORY:
            {
                hadError = true;
                al_log(true, "OpenAL: Out of memory\n");
            }
            break;
        }
    }
    return hadError;
}

internal bool32
al_log(bool32 error, const char *message, ...)
{
    va_list argptr;
    va_start(argptr, message);
    log_to_file(AL_LOG_FILE, message, argptr);
    va_end(argptr);
    if (error)
    {
        va_start(argptr, message);
        log_to_console(AL_LOG_FILE, message, argptr);
        va_end(argptr);
    }
    return true;
}

internal bool32 // NOTE: Returns true if there is an error
    al_print_if_error(char *message)
{
    if (!handle_alGetError()) return false;
    al_log(true, "OpenAL Error: %s\n", (message ? message : "no message"));
    return true;
}

internal bool32
al_is_playing(ALuint soundID)
{
    int32 state = 0;
    alGetSourcei(soundID, AL_SOURCE_STATE, &state);
    if (al_print_if_error("Failed to query sound-source's state\n")) return false;
    return (state == AL_PLAYING);
}

internal void
al_set_default_source_values(uint32 sourceID)
{
    alSourcef(sourceID, AL_PITCH, 1);
    alSourcef(sourceID, AL_GAIN, 0.5f);
    alSource3f(sourceID, AL_POSITION, 0, 0, 0);
    alSource3f(sourceID, AL_VELOCITY, 0, 0, 0);
    alSourcei(sourceID, AL_LOOPING, AL_FALSE);
    al_print_if_error("Failed to set default source values\n");
}

internal uint32
al_add_sound(char *filename, sound_array *sounds)
{
    if (sounds->next >= sounds->size - 1)
    {
        al_log(true, "Failed to add new sound %s : At maximum number of sounds\n", filename);
        return 0;
    }
    if (!al_load_sound_file(&sounds->sources[sounds->next], &sounds->buffers[sounds->next], filename))
    {
        al_log(true, "Failed to add new sound %s\n", filename);
        return 0;
    }
    return sounds->sources[sounds->next++];
}

internal bool32
al_load_sound_file(uint32 *sourceID, uint32 *bufferID, char *filename)
{
    if ((*bufferID = alureCreateBufferFromFile(filename)) == AL_NONE)
    {
        al_log(true, "Alure failed to load %s. Alure error: %s\n", filename, alureGetErrorString());
        return false;
    }
    if (al_print_if_error("AL load sound: Failed to generate audio buffer\n")) return false;
    alSourcei(*sourceID, AL_BUFFER, *bufferID);
    if (al_print_if_error("AL load sound: Failed to bind source to buffer\n")) return false;
    al_set_default_source_values(*sourceID);
    return true;
}

internal bool32
initialize_sound(sound_array *sounds)
{
    ALCdevice *device = g_audioDevice = alcOpenDevice(NULL);
    if (device)
    {
        g_audioContext = alcCreateContext(device, NULL);
        if (alcMakeContextCurrent(g_audioContext))
        {
            if (!start_new_log(AL_LOG_FILE))
            {
                fprintf(stderr, "Failed to create new log file: %s\n", AL_LOG_FILE);
            }

            alListenerfv(AL_POSITION, g_listenerPos);
            alListenerfv(AL_VELOCITY, g_listenerVel);
            alListenerfv(AL_ORIENTATION, g_listenerOri);
            if (al_print_if_error("Failed to initialize listener\n")) return false;

            alGenSources(sounds->size, sounds->sources);
            if (al_print_if_error("Failed to generate audio sources\n")) return false;
        }
        else
        {
            al_log(true, "OpenAL: failed to make audio context current\n");
            return false;
        }
    }
    else
    {
        al_log(true, "Failed to open device\n");
        // TODO: fix bug where alcOpenDevice occasionally fails
        return false;
    }
    return true;
}

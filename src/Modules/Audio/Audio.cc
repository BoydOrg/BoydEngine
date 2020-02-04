#include "../../Components/AudioSource.hh"
#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

#include <AL/al.h>
#include <AL/alc.h>
#include <tinywav.h>

/// TODO: add state transfer
struct BoydAudioState
{
    ALCcontext *context;
    ALCdevice *device;
    bool isEnabled{true};

    BoydAudioState()
    {
        if(!(device = alcOpenDevice(nullptr)))
        {
            BOYD_LOG(Error, "Could not open the default device for audio sink. Audio will be muted.");
            isEnabled = false;
        }
        else if(!(context = alcCreateContext(device, nullptr)))
        {
            BOYD_LOG(Error, "Could not create an OpenAL context. Audio will be muted.");
            isEnabled = false;
        }
    }
    ~BoydAudioState()
    {
        if(context)
        {
            alcDestroyContext(context);
        }
        if(device)
        {
            alcCloseDevice(device);
        }
    }
};

inline BoydAudioState *GetState(void *state)
{
    return reinterpret_cast<BoydAudioState *>(state);
}

void OnAudioSourceRegister(entt::entity entity, const entt::registry &registry)
{
    //auto &audioSource = registry.get<boyd::comp::AudioSource>(entity);
}

extern "C" {
BOYD_API void *BoydInit_Audio()
{
    BOYD_LOG(Info, "Starting Audio module");
    //auto &registry = Boyd_GameState()->ecs;
    //registry.on_construct<boyd::comp::AudioSource>().connect<OnAudioSourceRegister>();
    return new BoydAudioState;
}

BOYD_API void BoydUpdate_Audio(void *state)
{
}

BOYD_API void BoydHalt_Audio(void *state)
{
    BOYD_LOG(Info, "Halting AudioState module");
    //auto &registry = Boyd_GameState()->ecs;
    //registry.on_construct<boyd::comp::AudioSource>().disconnect<OnAudioSourceRegister>();
    delete GetState(state);
}
}
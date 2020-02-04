#include "../../Components/AudioSource.hh"
#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

#include "Utils.hh"

#include <AL/al.h>
#include <AL/alc.h>
#include <filesystem>

using namespace boyd;

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
            BOYD_LOG(Error, "Could not access the device");
            PrintOpenALCError(device);
            BOYD_LOG(Error, "Audio will be disabled");
            isEnabled = false;
        }
        else if(!(context = alcCreateContext(device, nullptr)) || alcMakeContextCurrent(context) == ALC_FALSE)
        {
            BOYD_LOG(Error, "Could not create an OpenAL context. Audio will be muted.");
            PrintOpenALCError(device);
            BOYD_LOG(Error, "Audio will be disabled");
            isEnabled = false;
        }
        else
        {
            // Load some extensions
            const char *name = nullptr;
            if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
                name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
            if(!name || alcGetError(device) != AL_NO_ERROR)
                name = alcGetString(device, ALC_DEVICE_SPECIFIER);

            BOYD_LOG(Debug, "OpenAL extension detected: {}", name);
        }
    }
    ~BoydAudioState()
    {
        BOYD_LOG(Debug, "Destroying OpenAL context");
        alcMakeContextCurrent(nullptr);
        if(context)
        {
            alcDestroyContext(context);
        }
        BOYD_LOG(Debug, "Closing OpenAL device");
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

void OnAudioSourceRegister(entt::registry &registry, entt::entity entity)
{
    auto &audioSource = registry.get<boyd::comp::AudioSource>(entity);

    std::filesystem::path assetFile{audioSource.assetFile};
    auto extension = assetFile.extension().string();

    BOYD_LOG(Info, "Loading audio asset {}", assetFile.string());

    if(extension == ".wav")
    {
        boyd::LoadWav(assetFile, audioSource);
    }
    else if(extension == ".flac")
    {
        boyd::LoadFlac(assetFile, audioSource);
    }
    else
    {
        BOYD_LOG(Warn, "Unknown audio format: {}", assetFile.string());
    }
}

void OnAudioSourceDeregister(entt::registry &registry, entt::entity entity)
{
    auto &audioSource = registry.get<boyd::comp::AudioSource>(entity);

    alDeleteSources(1, &audioSource.alSource);
    alDeleteBuffers(1, &audioSource.alBuffer);
}

extern "C" {
BOYD_API void *BoydInit_Audio()
{
    BOYD_LOG(Info, "Starting Audio module");
    auto *audioState = new BoydAudioState();
    auto &registry = Boyd_GameState()->ecs;
    registry.on_construct<boyd::comp::AudioSource>().connect<OnAudioSourceRegister>();
    registry.on_destroy<boyd::comp::AudioSource>().connect<OnAudioSourceDeregister>();

    return audioState;
}

BOYD_API void BoydUpdate_Audio(void *state)
{
    auto *AudioState = GetState(state);
    if(!AudioState->isEnabled)
    {
        return;
    }
}

BOYD_API void BoydHalt_Audio(void *state)
{
    BOYD_LOG(Info, "Halting AudioState module");
    auto &registry = Boyd_GameState()->ecs;
    registry.on_construct<boyd::comp::AudioSource>().disconnect<OnAudioSourceRegister>();
    delete GetState(state);
}
}
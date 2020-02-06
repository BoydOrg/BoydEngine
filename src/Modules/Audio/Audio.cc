#include "../../Components/AudioSource.hh"
#include "../../Components/Camera.hh"
#include "../../Components/Transform.hh"
#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

#include "Utils.hh"

#include <AL/al.h>
#include <AL/alc.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <vector>

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
            BOYD_OPENALC_ERROR(device);
            BOYD_LOG(Error, "Audio will be disabled");
            isEnabled = false;
        }
        else if(!(context = alcCreateContext(device, nullptr)) || alcMakeContextCurrent(context) == ALC_FALSE)
        {
            BOYD_LOG(Error, "Could not create an OpenAL context. Audio will be muted.");
            BOYD_OPENALC_ERROR(device);
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
    entt::registry &registry = Boyd_GameState()->ecs;
    if(!AudioState->isEnabled)
    {
        // Delete all the audiosources. The worst case is a memory leak of audio sources
        auto view = registry.view<boyd::comp::AudioSource>();
        registry.destroy(view.begin(), view.end());
    }
    else
    {
        auto view = registry.view<boyd::comp::AudioSource, boyd::comp::Transform>();

        boyd::comp::Camera *camera;

        registry.view<boyd::comp::Camera>().each([&camera](entt::entity entity, auto &cameraComp) { camera = &cameraComp; });
        /// TODO: Replace camera.position with the corresponding transform
        alListenerfv(AL_POSITION, (const ALfloat *)&(camera->camera.target));
        BOYD_OPENAL_ERROR();

        Vector3 orientation[] = {camera->camera.target, camera->camera.up};
        alListenerfv(AL_ORIENTATION, (const ALfloat *)orientation);
        BOYD_OPENAL_ERROR();

        std::vector<entt::entity> flushPool;

        view.each([&flushPool](entt::entity entity, boyd::comp::AudioSource &audioSource, boyd::comp::Transform &transform) {
            /// if the audio element is a SFX and the reproduction stream has run out, delete it.
            ALenum state;

            alGetSourcei(audioSource.alSource, AL_SOURCE_STATE, &state);
            BOYD_OPENAL_ERROR();

            if(audioSource.soundType == boyd::comp::AudioSource::SoundType::SFX && state == AL_STOPPED)
            {
                BOYD_LOG(Debug, "Evicting asset {}", audioSource.assetFile);
                flushPool.push_back(entity);
            }
            else if(audioSource.soundType != boyd::comp::AudioSource::SoundType::BGM)
            {
                glm::vec3 translation = glm::vec3(transform.matrix[3]);

                // Position is given only by the translation right now
                // Also, Audio sources are omnidirectional
                alSourcefv(audioSource.alSource, AL_POSITION, (const ALfloat *)&translation);
                BOYD_OPENAL_ERROR();
                alSourcefv(audioSource.alSource, AL_DIRECTION, (const ALfloat *)(float[3]){0, 0, 0});
                BOYD_OPENAL_ERROR();
            }
        });

        for(auto entity : flushPool)
            registry.destroy(entity);
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
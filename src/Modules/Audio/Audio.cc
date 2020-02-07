#include "../../Components/AudioClip.hh"
#include "../../Components/AudioInternals.hh"
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
#include <tuple>
#include <vector>

#include <type_traits>
using namespace boyd;

/// TODO: add state transfer
struct BoydAudioState
{
    ALCcontext *context;
    ALCdevice *device;

    entt::observer entt_clipAndSource;

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
            // Load all available extensions, similarly to GLEW for OpenGL.
            const char *name = nullptr;
            if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
                name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
            if(!name || alcGetError(device) != AL_NO_ERROR)
                name = alcGetString(device, ALC_DEVICE_SPECIFIER);

            BOYD_LOG(Debug, "OpenAL extension detected: {}", name);

            auto &registry = Boyd_GameState()->ecs;

            entt_clipAndSource.connect(registry, entt::collector.group<boyd::comp::AudioClip,
                                                                       boyd::comp::AudioSource>());
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

extern "C" {
BOYD_API void *BoydInit_Audio()
{
    BOYD_LOG(Info, "Starting Audio module");
    return new BoydAudioState();
}

BOYD_API void BoydUpdate_Audio(void *state)
{
    auto *audioState = GetState(state);
    entt::registry &registry = Boyd_GameState()->ecs;

    alcMakeContextCurrent(audioState->context);
    BOYD_OPENALC_ERROR(audioState->device);

    auto entt_clipAndSourceView = registry.view<boyd::comp::AudioClip, boyd::comp::AudioSource>();
    auto entt_transform = registry.view<boyd::comp::AudioSource, boyd::comp::AudioInternals, boyd::comp::Transform>();

    if(!audioState->isEnabled)
    {
        // Delete all the audiosources. Keep the ECS clean
        auto view = registry.view<boyd::comp::AudioSource>();
        registry.destroy(view.begin(), view.end());
    }
    else
    {
        // Register new pairs <AudioSource, AudioClip> if any
        for(const auto entity : audioState->entt_clipAndSource)
        {
            auto tuple = entt_clipAndSourceView.get<boyd::comp::AudioClip, boyd::comp::AudioSource>(entity);
            boyd::comp::AudioClip &clip = std::get<0>(tuple);
            boyd::comp::AudioSource &source = std::get<1>(tuple);
            boyd::comp::AudioInternals &internals = registry.get_or_assign<boyd::comp::AudioInternals>(entity, clip);

            if(!internals.isSet)
            {
                alGenBuffers(1, &internals.dataBuffer);
                BOYD_OPENAL_ERROR();
                alBufferData(internals.dataBuffer, internals.format,
                             clip.wave.data.get(), clip.wave.sampleCount * clip.wave.channels * clip.wave.sampleSize / 8,
                             clip.wave.sampleRate);
                BOYD_OPENAL_ERROR();
                alGenSources(1, &internals.source);
                BOYD_OPENAL_ERROR();
                alSourcei(internals.source, AL_BUFFER, internals.dataBuffer);
                BOYD_OPENAL_ERROR();

                switch(source.soundType)
                {
                case boyd::comp::AudioSource::SoundType::SFX_LOOPABLE:
                case boyd::comp::AudioSource::SoundType::BGM:
                    alSourcei(internals.source, AL_LOOPING, AL_TRUE);
                    break;
                case boyd::comp::AudioSource::SoundType::SFX:
                    alSourcei(internals.source, AL_LOOPING, AL_FALSE);
                }
                BOYD_OPENAL_ERROR();
                alSourcePlay(internals.source);
                BOYD_OPENAL_ERROR();

                internals.isSet = true;
            }
        }

        boyd::comp::Camera *camera = nullptr;
        registry.view<boyd::comp::Camera>().each([&camera](entt::entity entity, auto &cameraComp) { camera = &cameraComp; });

        /// TODO: Replace camera.position with the corresponding transform
        if(camera)
        {
            // FIXME - get camera position and orientation from its transform!
            //
            //alListenerfv(AL_POSITION, (const ALfloat *)&(camera->camera.position));
            //BOYD_OPENAL_ERROR();
            //
            //Vector3 orientation[] = {camera->camera.target, camera->camera.up};
            //alListenerfv(AL_ORIENTATION, (const ALfloat *)orientation);
            //BOYD_OPENAL_ERROR();
        }

        std::vector<entt::entity> flushPool;

        entt_transform.each([&flushPool](entt::entity entity, auto &source, auto &internals, auto &transform) {
            /// If the audio element is a SFX and the reproduction stream has run out, delete it.
            ALenum state;

            alGetSourcei(internals.source, AL_SOURCE_STATE, &state);

            if(source.soundType == boyd::comp::AudioSource::SoundType::SFX && state == AL_STOPPED)
            {
                flushPool.push_back(entity);
            }
            else if(source.soundType != boyd::comp::AudioSource::SoundType::BGM)
            {
                /// ENRICO: strangely my compiler (GCC 9.2.0) decided to crash if I tried to create
                /// the arrays on the fly. I did not submit a report though 'cause I am a bad kid.
                glm::vec3 translation = glm::vec3(transform.matrix[3]);
                ALfloat zeroArray[3] = {0.0f, 0.0f, 0.0f};

                // Position is given only by the translation right now
                alSourcefv(internals.source, AL_POSITION, (const ALfloat *)&translation);
                BOYD_OPENAL_ERROR();
                alSourcefv(internals.source, AL_DIRECTION, zeroArray);
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
    delete GetState(state);
}
}

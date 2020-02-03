#include "../../Components/AudioSource.hh"
#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

#include <AL/al.h>
#include <AL/alc.h>
#include <filesystem>
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
        TinyWav tw;
        if(!tinywav_open_read(&tw, assetFile.string().c_str(), TW_SPLIT, TW_FLOAT32))
        {
            BOYD_LOG(Warn, "Could not load audio file: {}", assetFile.string());
        }
        else
        {
            int16_t numChannels = tw.numChannels;
            uint32_t dataSize = tw.h.Subchunk2Size;
            uint32_t bitDepth = tw.h.Subchunk1Size;
            auto frames = dataSize / (numChannels * bitDepth / 16);

            uint32_t frequency = tw.h.SampleRate;

            ALenum format;
            if(bitDepth == 16)
            {
                switch(numChannels)
                {
                case 1:
                    format = AL_FORMAT_MONO16;
                    break;
                case 2:
                    format = AL_FORMAT_STEREO16;
                    break;
                default:
                    BOYD_LOG(Warn, "Unrecognized format option: {} channels and {} bit of depth", numChannels, bitDepth);
                }
            }
            else
            {
                switch(numChannels)
                {
                case 1:
                    format = AL_FORMAT_MONO8;
                    break;
                case 2:
                    format = AL_FORMAT_STEREO16;
                    break;
                default:
                    BOYD_LOG(Warn, "Unrecognized format option: {} channels and {} bit of depth", numChannels, bitDepth);
                }
            }

            float data[numChannels][frames];

            tinywav_read_f(&tw, data, frames);

            alGenBuffers(1, &audioSource.buffer);
            alBufferData(audioSource.buffer, format, (void *)data, dataSize, tw.h.SampleRate);

            tinywav_close_read(&tw);

            alGenSources(1, &audioSource.alSource);

            switch(audioSource.soundType)
            {
            case boyd::comp::AudioSource::SoundType::SFX:
                alSourcei(audioSource.alSource, AL_LOOPING, AL_TRUE);
                break;
            default:
                alSourcei(audioSource.alSource, AL_LOOPING, AL_FALSE);
                break;
            }

            alSourcePlay(audioSource.alSource);
        }
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
    alDeleteBuffers(1, &audioSource.buffer);
}

extern "C" {
BOYD_API void *BoydInit_Audio()
{
    BOYD_LOG(Info, "Starting Audio module");
    auto &registry = Boyd_GameState()->ecs;
    registry.on_construct<boyd::comp::AudioSource>().connect<OnAudioSourceRegister>();
    registry.on_destroy<boyd::comp::AudioSource>().connect<OnAudioSourceDeregister>();

    return new BoydAudioState;
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
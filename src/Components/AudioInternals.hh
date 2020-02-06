#pragma once

#include "../Debug/Log.hh"
#include "AudioClip.hh"
#include <AL/al.h>
#include <AL/alc.h>

namespace boyd
{
namespace comp
{

struct BOYD_API AudioInternals
{
    ALuint dataBuffer;
    ALuint source;

    explicit AudioInternals(const AudioClip &clip)
    {

        auto &wave = *clip.wave;
        int channels = wave.channels;
        int sampleRate = wave.sampleCount;
        int bitsPerSample = wave.sampleSize;

        ALenum format;
        if(channels == 1 && bitsPerSample == 8)
            format = AL_FORMAT_MONO8;
        else if(channels == 1 && bitsPerSample == 16)
            format = AL_FORMAT_MONO16;
        else if(channels == 2 && bitsPerSample == 8)
            format = AL_FORMAT_STEREO8;
        else if(channels == 2 && bitsPerSample == 16)
            format = AL_FORMAT_STEREO16;
        else
        {
            BOYD_LOG(Warn, "OpenAL does not support this file. Got bps: {}; channels: {}", bitsPerSample, channels);
            return;
        }

        alGenBuffers(1, &dataBuffer);
        alBufferData(dataBuffer, format, wave.data, wave.sampleCount * channels * bitsPerSample / 8, sampleRate);
        alGenSources(1, &source);
        alSourcei(source, AL_BUFFER, dataBuffer);
    }

    ~AudioInternals()
    {
        alDeleteBuffers(1, &source);
        alDeleteBuffers(1, &dataBuffer);
    }

    /// Internals should not be shared.
    AudioInternals() = delete;
    AudioInternals(const AudioInternals &) = delete;
    AudioInternals &operator=(const AudioInternals &) = delete;
    AudioInternals(AudioInternals &&o) = default;
    AudioInternals &operator=(AudioInternals &&) = default;
};

} // namespace comp
} // namespace boyd
#pragma once

#include "../Core/Platform.hh"
#include <memory>

namespace boyd
{

struct BOYD_API Wave
{
    unsigned int sampleCount;      // Total number of samples
    unsigned int sampleRate;       // Frequency (samples per second)
    unsigned int sampleSize;       // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
    unsigned int channels;         // Number of channels (1-mono, 2-stereo)
    std::shared_ptr<uint8_t> data; // Buffer data pointer
};

namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API AudioClip
{
    Wave wave; ///< The internal audio representation.

    /// Creates a null audioclip.
    AudioClip() = default;

    /// Creates an audioclip from the given Wave.
    /// WARNING: It will be automatically `UnloadWave()`d by the component!
    AudioClip(Wave &&wave)
        : wave{wave}
    {
    }

    AudioClip(const AudioClip &toCopy) = default;
    AudioClip &operator=(const AudioClip &toCopy) = default;
    AudioClip(AudioClip &&toMove) = default;
    AudioClip &operator=(AudioClip &&toMove) = default;
};

} // namespace comp
} // namespace boyd

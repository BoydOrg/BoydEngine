#pragma once

#include "../Core/Platform.hh"
#include <memory>
#include <raylib.h>

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API AudioClip
{
    std::shared_ptr<::Wave> wave; ///< The internal audio representation.

    /// Creates a null audioclip.
    AudioClip()
        : wave{nullptr}
    {
    }
    /// Creates an audioclip from the given Wave.
    /// WARNING: It will be automatically `UnloadWave()`d by the component!
    explicit AudioClip(::Wave wave)
        : wave{new ::Wave{std::move(wave)}, WaveDeleter}
    {
    }

    AudioClip(const AudioClip &toCopy) = default;
    AudioClip &operator=(const AudioClip &toCopy) = default;
    AudioClip(AudioClip &&toMove) = default;
    AudioClip &operator=(AudioClip &&toMove) = default;

private:
    static void WaveDeleter(Wave *wave)
    {
        ::UnloadWave(*wave);
        wave->data = nullptr;
    }
};

} // namespace comp
} // namespace boyd

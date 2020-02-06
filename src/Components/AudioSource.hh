#pragma once

#include "../Core/Platform.hh"
#include <AL/al.h>
#include <AL/alc.h>
#include <glm/glm.hpp>
#include <string>

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API AudioSource
{
    std::string assetFile{};

    enum class SoundType
    {
        SFX,          /// For normal sound effects that should be located in space.
        SFX_LOOPABLE, /// For looping sound effects that should be located in space.
        BGM,          /// For looping background music that should be omnidirectional.
    } soundType;

    AudioSource(std::string assetFile, SoundType soundType)
        : assetFile{assetFile}, soundType{soundType}
    {
    }
    /// Internal OpenAL buffer, do not edit!
    ALuint alBuffer;
    ALuint alSource;
};

} // namespace comp
} // namespace boyd
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
        SFX,
        BGM
    } soundType;

    AudioSource(std::string assetFile, SoundType soundType)
    : assetFile{assetFile}, soundType{soundType}
    {}
    /// Internal OpenAL buffer, do not edit!
    ALuint buffer;
    ALuint alSource;
};

} // namespace comp
} // namespace boyd
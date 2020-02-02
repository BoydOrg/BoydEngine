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
    std::string audioSource{};

    // TODO: add Alsoft internals
    ALuint source;

    enum class SoundType
    {
        SFX,
        BGM
    };
};

} // namespace comp
} // namespace boyd
#pragma once

#include "../Core/Platform.hh"
#include "../Scripting/Lua.hh"
#include <AL/al.h>
#include <AL/alc.h>
#include <glm/glm.hpp>

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API AudioSource
{
    string audioSource{};

    // TODO: add Alsoft internals
    ALuint source;

    enum class SoundType
    {
        SFX,
        BGM
    };
};

template <>
struct ScriptRegistrar<Mesh>
{
    register()
};

} // namespace comp
} // namespace boyd